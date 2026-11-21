// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/BaseSpear.h"
#include "Weapons/WeaponData.h"
#include "Components/BoxComponent.h"
#include "Actor/UC_Player.h"
#include "Weapons/ProjectileBase.h"
#include "Debug/DebugOverlaySubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpear, Log, All);

ABaseSpear::ABaseSpear()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	BoxCollision = CreateDefaultSubobject<UBoxComponent>("BoxCollision");
	BoxCollision->SetGenerateOverlapEvents(false);
	BoxCollision->SetupAttachment(Mesh);

	bAbsoluteRotation = true;
}

void ABaseSpear::BeginPlay()
{
	Super::BeginPlay();

	// Delegate Binding
	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ABaseSpear::ApplyDamageCollision);

	if (UHordeSubsystem* H = GetWorld()->GetSubsystem<UHordeSubsystem>())
	{
		Horde = H;
	}
	else UE_LOG(LogSpear, Error, TEXT("Failed to retrieve HordeSubystem in ABaseBowToGun!"));

	// Thrust Properties
	if (Data)
	{
		ProjectileSpeed  = Data->ProjectileSpeed;
		ProjectileRange  = Data->ProjectileRange;
		ProjectilePierce = Data->ProjectilePierce;
		EvolutionLevel   = Data->EvolutionLevel;
		Cooldown         = Data->Cooldown;
		Height           = Data->Height;
		Radius           = Data->DistanceFromPlayer;
		Range            = Data->Range + Radius;
		Speed            = Data->AttackSpeed;
	}
	else
	{
		UE_LOG(LogSpear, Error, TEXT("ABaseSpear's WeaponData asset was not set!"));

		ProjectileSpeed  = 0.f;
		ProjectileRange  = 0.f;
		ProjectilePierce = 0.f;
		EvolutionLevel   = 0.f;
		Cooldown         = 0.f;
		Height           = 0.f;
		Radius           = 0.f;
		Range            = 0.f;
		Speed            = 0.f;
	}

	if (EvolutionLevel > 0)
	{
		OnWeaponAttack.AddDynamic(this, &ABaseSpear::JavelinRain);
	}

	bIsAiming = true;
}

void ABaseSpear::Tick(float DeltaTime)
{
	// We only aim while we're not thrusting
	if (bIsAiming)
	{
		ABaseBowToGun::FaceNearestEnemy(Pivot, Horde, (AActor*)Parent, Range, true/*, false*/);
		return;
	}

	// Thrusting movement
	float CurrentDistance = MoveSpear(DeltaTime);

	// Did we reach our range?
	if (bIsThrusting && CurrentDistance > Range)
	{
		Retreat(); // Yes, we fully thrusted our spear
	}
	else if (CurrentDistance < Radius)
	{
		EndCycle(); // Yes, we fully retreated our spear 
	}
	// else we continue the movement
}

void ABaseSpear::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopWeapon();

	Super::EndPlay(EndPlayReason);
}

float ABaseSpear::MoveSpear(float DeltaTime)
{
	Mesh->AddRelativeLocation(FVector(0.f, (bIsThrusting) ? (Speed * ParentStats->GetAttackSpeed()) * DeltaTime : -(Speed * ParentStats->GetAttackSpeed()) * RetreatMultiplier * DeltaTime, 0.f));
	return Mesh->GetRelativeLocation().Y;
}

void ABaseSpear::Thrust()
{
	bIsAiming = false;
	bIsThrusting = true;
	OnWeaponAttack.Broadcast();

	BoxCollision->SetGenerateOverlapEvents(true);
}

void ABaseSpear::Retreat()
{
	bIsThrusting = false;
	Mesh->SetRelativeLocation(FVector(0.f, Range, Height));
	BoxCollision->SetGenerateOverlapEvents(false);
}

void ABaseSpear::EndCycle()
{
	HitActors.Empty();

	bIsAiming = true;
	bIsThrusting = true;
	Mesh->SetRelativeLocation(FVector(0.f, Radius, Height));

	GetWorldTimerManager().SetTimer(TThrustCooldown, this, &ABaseSpear::Thrust, Cooldown);
}


void ABaseSpear::ApplyDamageCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsThrusting && !HitActors.Contains(OtherActor))
	{
		HitActors.Add(OtherActor);
		ApplyDamage(this, OtherActor);
	}
}

void ABaseSpear::StartWeapon()
{
	Super::StartWeapon();

	EndCycle();
}

void ABaseSpear::StopWeapon()
{
	HitActors.Empty();
	GetWorldTimerManager().ClearTimer(TThrustCooldown);

	Super::StopWeapon();
}

void ABaseSpear::JavelinRain()
{
	FVector SpawnLocation = GetActorLocation();

	// Missing parent: still return a valid rotation
	FRotator SpawnDirection = IsValid(Parent) ? Parent->GetActorRotation() : GetActorRotation();

	FProjectileLaunchParams Params;
	Params.Origin = SpawnLocation;
	Params.Speed  = ProjectileSpeed * ParentStats->GetAttackSpeed();
	Params.Pierce = ProjectilePierce;
	Params.Range  = ProjectileRange;

	FOnProjectileHit Hit;
	Hit.BindUFunction(this, "ApplyDamage");

	// Launch as x instead of +
	if (bAlternation && ++JavelinAlternation % 2)
	{
		SpawnDirection.Add(0.f, 45.f, 0.f);
	}

	UWorld* World = GetWorld();
	// Launch at the four different directions
	for (int i = 0; i < 360; i += 90)
	{
		SpawnDirection.Add(0.f, 90.f, 0.f);
		Params.Direction = SpawnDirection.Vector(); // AProjectileBase::Fire normalizes the direction
		AProjectileBase::Fire(World, Data->ProjectileType, this, Params, Hit);
	}
}