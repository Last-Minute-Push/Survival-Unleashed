// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/PitcherPlant.h"
#include "Weapons/ProjectileBase.h"
#include "Framework/SUCollision.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

APitcherPlant::APitcherPlant()
{
	// Rooted plants shouldn't be shoved around by the crowd, and burrowed ones have no business steering around it
	SeparationWeight = 0.f;

	MoundMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MoundMesh"));
	MoundMesh->SetupAttachment(GetRootComponent());
	MoundMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MoundMesh->SetGenerateOverlapEvents(false);
}

void APitcherPlant::BeginPlay()
{
	Super::BeginPlay();

	// Pooled actors run BeginPlay once. Remember every mesh attached directly to the root (not the mound),
	// so sinking moves each piece once even if the Blueprint nests more meshes under them.
	TArray<UMeshComponent*> Meshes;
	GetComponents(Meshes);
	for (UMeshComponent* MeshComponent : Meshes)
	{
		if (MeshComponent != MoundMesh && MeshComponent->GetAttachParent() == GetRootComponent())
		{
			Visuals.Add({ MeshComponent, static_cast<float>(MeshComponent->GetRelativeLocation().Z) });
		}
	}

	if (AcidClass)
	{
		AProjectileBase::Prewarm(GetWorld(), AcidClass);
	}
}

void APitcherPlant::OnPoolAcquire_Implementation()
{
	Super::OnPoolAcquire_Implementation();

	// Every plant arrives underground and tunnels in from the spawn ring
	bHasBurrowTarget = false;
	RetargetTimer = 0.f;
	RepositionCooldownRemaining = 0.f;
	OutOfRangeTimer = 0.f;
	SetState(EState::Burrowed);
}

float APitcherPlant::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	// Area attacks and status ticks that don't go through collision still can't reach it underground
	if (State == EState::Burrowed)
	{
		return 0.f;
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

bool APitcherPlant::IsTargetable() const
{
	return State != EState::Burrowed && Super::IsTargetable();
}

FVector APitcherPlant::GetDesiredDirection() const
{
	if (State != EState::Burrowed || !bHasBurrowTarget)
	{
		return FVector::ZeroVector; // rooted, or coming up or going down
	}
	FVector ToTarget = BurrowTarget - GetActorLocation();
	ToTarget.Z = 0.f;
	return ToTarget.GetSafeNormal();
}

void APitcherPlant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead())
	{
		return;
	}

	// The pool may switch collision back on after OnPoolAcquire, so keep it matched to the state
	if (GetActorEnableCollision() == (State == EState::Burrowed))
	{
		ApplyCollision();
	}

	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return;
	}

	StateTime += DeltaTime;
	RepositionCooldownRemaining -= DeltaTime;

	switch (State)
	{
	case EState::Burrowed:
		RetargetTimer -= DeltaTime;
		TickBurrowed(Player->GetActorLocation());
		break;

	case EState::Surfacing:
		if (StateTime >= SurfaceTime)
		{
			SetState(EState::Rooted);
		}
		break;

	case EState::Rooted:
		TickRooted(Player, DeltaTime);
		break;

	case EState::Burrowing:
		if (StateTime >= BurrowTime)
		{
			SetState(EState::Burrowed);
		}
		break;
	}

	UpdateVisuals();
}

void APitcherPlant::TickBurrowed(const FVector& PlayerLocation)
{
	// Re-check the target now and then: the player moves, and a root wall may have gone up on it
	if (!bHasBurrowTarget || RetargetTimer <= 0.f)
	{
		RetargetTimer = 1.f;
		if (bHasBurrowTarget)
		{
			const float FromPlayer = FVector::Dist2D(BurrowTarget, PlayerLocation);
			FVector Ground;
			const bool bStillGood = FromPlayer >= PreferredRangeMin - 200.f && FromPlayer <= PreferredRangeMax + 200.f
				&& IsSpotClear(BurrowTarget, Ground);
			bHasBurrowTarget = bStillGood;
		}
		if (!bHasBurrowTarget)
		{
			PickBurrowTarget(PlayerLocation);
		}
	}

	if (bHasBurrowTarget && FVector::DistSquared2D(GetActorLocation(), BurrowTarget) < FMath::Square(60.f))
	{
		SetState(EState::Surfacing);
	}
}

void APitcherPlant::TickRooted(const APawn* Player, float DeltaTime)
{
	const FVector PlayerLocation = Player->GetActorLocation();
	const float Distance = FVector::Dist2D(GetActorLocation(), PlayerLocation);

	// Turn the mouth toward the player
	const float TargetYaw = (PlayerLocation - GetActorLocation()).Rotation().Yaw;
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), FRotator(0.f, TargetYaw, 0.f), DeltaTime, TurnSpeed));

	// Too close, or out of reach for too long: dig in and find a better spot
	OutOfRangeTimer = Distance > AcidRange ? OutOfRangeTimer + DeltaTime : 0.f;
	if (RepositionCooldownRemaining <= 0.f && (Distance < RepositionRange || OutOfRangeTimer >= OutOfRangeTime))
	{
		RepositionCooldownRemaining = RepositionCooldown;
		SetState(EState::Burrowing);
		return;
	}

	SpitCooldown -= DeltaTime;
	if (SpitCooldown <= 0.f && Distance <= AcidRange)
	{
		SpitAcid(Player);
		SpitCooldown = SpitInterval;
		SpitRecoverRemaining = SpitRecoverTime;
	}
	UpdateSpitPose(Distance <= AcidRange, DeltaTime);
}

void APitcherPlant::UpdateSpitPose(bool bInRange, float DeltaTime)
{
	float Pose = 0.f;

	// Just fired: lunging forward, easing back to rest
	if (SpitRecoverRemaining > 0.f)
	{
		SpitRecoverRemaining = FMath::Max(0.f, SpitRecoverRemaining - DeltaTime);
		Pose = SpitLungePose * (SpitRecoverRemaining / SpitRecoverTime);
	}
	// About to fire: lid opening, leaning back. Holds open if the player stepped out of range mid wind-up.
	else if (bInRange && SpitCooldown < SpitWindup)
	{
		Pose = 1.f - FMath::Max(SpitCooldown, 0.f) / SpitWindup;
	}

	SetMaterialData(EnemyMaterialData::AttackPose, Pose);
}

void APitcherPlant::SetState(EState NewState)
{
	State = NewState;
	StateTime = 0.f;

	if (State == EState::Burrowed)
	{
		bHasBurrowTarget = false; // pick a fresh spot on the next Tick
	}
	else if (State == EState::Rooted)
	{
		SpitCooldown = FirstSpitDelay;
		OutOfRangeTimer = 0.f;
	}
	// Any spit pose is dropped when it leaves the Rooted state (burrowing mid wind-up, for example)
	SpitRecoverRemaining = 0.f;
	SetMaterialData(EnemyMaterialData::AttackPose, 0.f);
	
	ApplyCollision();
	UpdateVisuals();
}

void APitcherPlant::ApplyCollision()
{
	// Off underground: no weapon overlaps, no projectile hits, no contact damage, and it tunnels under obstacles
	SetActorEnableCollision(State != EState::Burrowed);
}

void APitcherPlant::UpdateVisuals()
{
	// 0 = fully underground, 1 = fully up
	float Up = 1.f;
	switch (State)
	{
	case EState::Burrowed:  Up = 0.f; break;
	case EState::Surfacing: Up = FMath::InterpEaseOut(0.f, 1.f, FMath::Clamp(StateTime / SurfaceTime, 0.f, 1.f), 2.f); break;
	case EState::Rooted:    Up = 1.f; break;
	case EState::Burrowing: Up = 1.f - FMath::InterpEaseIn(0.f, 1.f, FMath::Clamp(StateTime / BurrowTime, 0.f, 1.f), 2.f); break;
	}

	for (const FVisual& Visual : Visuals)
	{
		if (USceneComponent* Component = Visual.Component.Get())
		{
			FVector Location = Component->GetRelativeLocation();
			Location.Z = Visual.RestZ - (1.f - Up) * SinkDepth;
			Component->SetRelativeLocation(Location);
		}
	}

	// The mound shows whenever any of the plant is below ground
	MoundMesh->SetVisibility(Up < 1.f);
}

bool APitcherPlant::PickBurrowTarget(const FVector& PlayerLocation)
{
	// Prefer the side of the player it's already on, so it doesn't tunnel straight past them
	const FVector FromPlayer = GetActorLocation() - PlayerLocation;
	const float BaseAngle = FMath::Atan2(FromPlayer.Y, FromPlayer.X);

	for (int32 Attempt = 0; Attempt < 10; ++Attempt)
	{
		const float Spread = FMath::DegreesToRadians(60.f + Attempt * 12.f); // widens if the near side is blocked
		const float Angle = BaseAngle + FMath::FRandRange(-Spread, Spread);
		const float Distance = FMath::FRandRange(PreferredRangeMin, PreferredRangeMax);
		const FVector Candidate = PlayerLocation + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Distance;

		FVector Ground;
		if (IsSpotClear(Candidate, Ground))
		{
			BurrowTarget = Ground;
			bHasBurrowTarget = true;
			return true;
		}
	}
	return false; // tries again in a second
}

bool APitcherPlant::IsSpotClear(const FVector& Point, FVector& OutGround) const
{
	FCollisionObjectQueryParams GroundOrObstacle(ECC_WorldStatic);
	GroundOrObstacle.AddObjectTypesToQuery(SUCollision::Obstacle);

	// Off the map: nothing underneath
	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByObjectType(Hit, Point + FVector(0.f, 0.f, 1000.f),
		Point - FVector(0.f, 0.f, 2000.f), GroundOrObstacle))
	{
		return false;
	}
	if (Hit.GetComponent() && Hit.GetComponent()->GetCollisionObjectType() == SUCollision::Obstacle)
	{
		return false; // on top of a pillar, log or root wall
	}

	// Room to stand: no obstacle within the capsule's radius
	const UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(GetRootComponent());
	const float Radius = (CapsuleComp ? CapsuleComp->GetScaledCapsuleRadius() : 50.f) + 20.f;
	if (GetWorld()->OverlapAnyTestByObjectType(Hit.ImpactPoint + FVector(0.f, 0.f, Radius), FQuat::Identity,
		FCollisionObjectQueryParams(SUCollision::Obstacle), FCollisionShape::MakeSphere(Radius)))
	{
		return false;
	}

	OutGround = Hit.ImpactPoint;
	return true;
}

void APitcherPlant::SpitAcid(const APawn* Player)
{
	if (!AcidClass)
	{
		return;
	}

	// Aimed at where the player is now, level, so a moving player can sidestep it
	const FVector Mouth = GetActorTransform().TransformPosition(MouthOffset);
	FVector Direction = Player->GetActorLocation() - Mouth;
	Direction.Z = 0.f;

	FProjectileLaunchParams Params;
	Params.Origin = Mouth;
	Params.Direction = Direction.GetSafeNormal();
	Params.Speed = AcidSpeed;
	Params.Range = AcidRange;
	Params.Pierce = 0;

	AProjectileBase::Fire(GetWorld(), AcidClass, this, Params,
		FOnProjectileHit::CreateUObject(this, &APitcherPlant::HandleAcidHit));
}

void APitcherPlant::HandleAcidHit(AProjectileBase* Projectile, AActor* Target)
{
	// Same path as contact damage, so Defense, Evasion and dash invincibility all apply
	UGameplayStatics::ApplyDamage(Target, AcidDamage, nullptr, this, nullptr);
}
