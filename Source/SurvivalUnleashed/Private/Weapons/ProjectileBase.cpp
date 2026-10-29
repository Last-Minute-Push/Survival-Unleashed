// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/ProjectileBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Framework/ObjectPoolSubsystem.h"
#include "Enemies/EnemyBase.h"
#include "Framework/SUCollision.h"

DEFINE_LOG_CATEGORY_STATIC(LogProjectile, Log, All);

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // only ticks while in flight

	Collision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	Collision->InitCapsuleSize(15.f, 100.f);
	Collision->SetCollisionProfileName(TEXT("PlayerAttack"));
	Collision->SetGenerateOverlapEvents(true);
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetGenerateOverlapEvents(false);
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	// BeginPlay runs once per actor, even when pooled, so this binds exactly once
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnCollisionBeginOverlap);
}

/*static*/ AProjectileBase* AProjectileBase::Fire(UWorld* World, TSubclassOf<AProjectileBase> ProjectileClass, AActor* OwningWeapon,
	const FProjectileLaunchParams& Params, FOnProjectileHit OnHit)
{
	UObjectPoolSubsystem* Pool = World ? World->GetSubsystem<UObjectPoolSubsystem>() : nullptr;
	if (!Pool || !ProjectileClass)
	{
		return nullptr;
	}

	const FTransform SpawnTransform(Params.Direction.Rotation(), Params.Origin);
	AProjectileBase* Projectile = Pool->AcquireAs<AProjectileBase>(ProjectileClass, SpawnTransform);
	if (Projectile)
	{
		Projectile->Launch(OwningWeapon, Params, MoveTemp(OnHit));
	}
	return Projectile;
}

/*static*/ void AProjectileBase::Prewarm(UWorld* World, TSubclassOf<AProjectileBase> ProjectileClass)
{
	UObjectPoolSubsystem* Pool = World ? World->GetSubsystem<UObjectPoolSubsystem>() : nullptr;
	if (Pool && ProjectileClass)
	{
		Pool->Prewarm(ProjectileClass, ProjectileClass->GetDefaultObject<AProjectileBase>()->PrewarmCount);
	}
}

void AProjectileBase::Launch(AActor* OwningWeapon, const FProjectileLaunchParams& Params, FOnProjectileHit OnHit)
{
	SetOwner(OwningWeapon);
	SetInstigator(OwningWeapon ? OwningWeapon->GetInstigator() : nullptr);
	HitCallback = MoveTemp(OnHit); // replaces any previous binding

	Direction = Params.Direction.GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = FVector::ForwardVector;
	}
	Speed = Params.Speed;
	RangeRemaining = Params.Range;
	HitsRemaining = Params.Pierce < 0 ? INDEX_NONE : Params.Pierce + 1;

	SetActorLocationAndRotation(Params.Origin, Direction.Rotation());

	bActive = true;
	bSpent = false;
	SetActorTickEnabled(true);

	if (Speed <= 0.f || RangeRemaining <= 0.f)
	{
		UE_LOG(LogProjectile, Warning, TEXT("%s launched with no speed or range"), *GetName());
		bSpent = true; // released next Tick instead of sitting in the world forever
		return;
	}

	// Enemies already touching the spawn point overlapped before Launch set up the callback,
	// so they never send a new begin-overlap. Hit them now.
	Collision->UpdateOverlaps(); // the pool enabled collision without moving us, so the overlap list may be stale
	TArray<AActor*> Touching;
	Collision->GetOverlappingActors(Touching, AEnemyBase::StaticClass());
	for (AActor* Target : Touching)
	{
		HandleHit(Target);
		if (bSpent)
		{
			break;
		}
	}
}

void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bActive)
	{
		return;
	}

	if (!bSpent)
	{
		const float Step = FMath::Min(Speed * DeltaTime, RangeRemaining);

		// Sweep so fast projectiles overlap every enemy along the path instead of skipping past them
		AddActorWorldOffset(Direction * Step, true);

		RangeRemaining -= Step;
		if (RangeRemaining <= KINDA_SMALL_NUMBER)
		{
			bSpent = true;
		}
	}

	// Release here, after the move, never from inside an overlap callback
	if (bSpent)
	{
		ReleaseToPool();
	}
}

void AProjectileBase::OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA<AEnemyBase>())
	{
		HandleHit(OtherActor);
	}
	else if (bActive && OtherComp && OtherComp->GetCollisionObjectType() == SUCollision::Obstacle)
	{
		bSpent = true; // stopped by a pillar, log, or wall
	}
}

void AProjectileBase::HandleHit(AActor* Target)
{
	if (!bActive || bSpent || !Target || AlreadyHit.Contains(Target))
	{
		return;
	}

	AlreadyHit.Add(Target);
	HitCallback.ExecuteIfBound(this, Target);

	if (HitsRemaining != INDEX_NONE && --HitsRemaining <= 0)
	{
		bSpent = true;
	}
}

void AProjectileBase::ReleaseToPool()
{
	if (!bActive)
	{
		return; // release exactly once
	}
	bActive = false;

	if (UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
	{
		Pool->Release(this);
	}
}

void AProjectileBase::OnPoolAcquire_Implementation()
{
	// Inactive until Launch; the pool has already placed us at the spawn transform
	bActive = false;
	bSpent = false;
	AlreadyHit.Reset();
	RangeRemaining = 0.f;
	HitsRemaining = 0;
}

void AProjectileBase::OnPoolRelease_Implementation()
{
	bActive = false;
	SetActorTickEnabled(false);
	HitCallback.Unbind();
	AlreadyHit.Reset();
	SetOwner(nullptr);
	SetInstigator(nullptr);
}

