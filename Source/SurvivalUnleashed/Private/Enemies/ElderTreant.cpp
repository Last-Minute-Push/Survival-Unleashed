// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/ElderTreant.h"
#include "Enemies/RootWall.h"
#include "Enemies/SpawnDirector.h"
#include "Framework/ObjectPoolSubsystem.h"
#include "Framework/SUCollision.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

namespace
{
	// Material parameter the slam decal can use to fill the ring during the telegraph
	const FName SlamProgressParam(TEXT("Progress"));

	// Space a summoned Vine Crawler needs clear of obstacles
	constexpr float SummonClearance = 60.f;
}

AElderTreant::AElderTreant()
{
	// Projects straight down onto the ground around the Treant
	SlamDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("SlamDecal"));
	SlamDecal->SetupAttachment(GetRootComponent());
	SlamDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	SlamDecal->SetVisibility(false);
}

void AElderTreant::BeginPlay()
{
	Super::BeginPlay();

	// Sized here, not in the constructor, so a Blueprint's SlamRadius is used.
	// DecalSize is a half size: X is depth, deep enough to reach the ground from the capsule's center.
	const UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(GetRootComponent());
	const float Depth = (CapsuleComp ? CapsuleComp->GetScaledCapsuleHalfHeight() : 200.f) + 100.f;
	SlamDecal->DecalSize = FVector(Depth, SlamRadius, SlamRadius);
	SlamDecal->MarkRenderStateDirty();
	SlamDecalMaterial = SlamDecal->CreateDynamicMaterialInstance();

	// Pooled actors run BeginPlay once, so this prewarms one set of walls for the whole run
	if (RootWallClass)
	{
		if (UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
		{
			Pool->Prewarm(RootWallClass, RootWallCount);
		}
	}
}

void AElderTreant::OnFightStarted()
{
	// Each attack comes after one full interval, so the opening seconds are just the walk-in
	SlamCooldown = SlamInterval;
	SummonCooldown = SummonInterval;
	RootWallCooldown = RootWallInterval;
	SlamWindupRemaining = 0.f;
	SetSlamTelegraph(false);
}

void AElderTreant::OnFightEnded()
{
	SlamWindupRemaining = 0.f;
	SetSlamTelegraph(false);
}

void AElderTreant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsFightActive() || IsDead())
	{
		return;
	}

	// Frozen or stunned: attacks wait too, so the elemental staves matter in the boss fight
	if (IsStatusActive(EEnemyStatus::Freeze) || IsStatusActive(EEnemyStatus::Stun))
	{
		return;
	}

	SlamCooldown -= DeltaTime;
	SummonCooldown -= DeltaTime;
	RootWallCooldown -= DeltaTime;

	if (IsSlamming())
	{
		SlamWindupRemaining -= DeltaTime;
		if (SlamWindupRemaining <= 0.f)
		{
			ResolveSlam();
		}
		else
		{
			SetSlamTelegraph(true, 1.f - SlamWindupRemaining / SlamTelegraph);
		}
		return; // one attack at a time; anything that came due waits for the slam to land
	}

	// At most one attack per frame. Walls first: they're the rarest and shape the next few seconds.
	if (RootWallCooldown <= 0.f)
	{
		RaiseRootWalls();
		RootWallCooldown = RootWallInterval;
	}
	else if (SummonCooldown <= 0.f)
	{
		SummonVines();
		SummonCooldown = SummonInterval;
	}
	else if (SlamCooldown <= 0.f)
	{
		StartSlam();
		SlamCooldown = SlamInterval;
	}
}

FVector AElderTreant::GetDesiredDirection() const
{
	// Plants its feet for the slam so the ring stays where it was shown
	return IsSlamming() ? FVector::ZeroVector : Super::GetDesiredDirection();
}

// ---------------------------------------------------------------- Ground Slam

void AElderTreant::StartSlam()
{
	if (SlamTelegraph <= 0.f)
	{
		ResolveSlam();
		return;
	}
	SlamWindupRemaining = SlamTelegraph;
	SetSlamTelegraph(true, 0.f);
}

void AElderTreant::ResolveSlam()
{
	SlamWindupRemaining = 0.f;
	SetSlamTelegraph(false);

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!Player)
	{
		return;
	}

	FVector Away = Player->GetActorLocation() - GetActorLocation();
	Away.Z = 0.f;
	if (Away.SizeSquared() > FMath::Square(SlamRadius))
	{
		return; // dodged
	}

	// Same path as contact damage, so Defense, Evasion and Thorns all apply
	UGameplayStatics::ApplyDamage(Player, SlamDamage, nullptr, this, nullptr);

	const FVector Direction = Away.IsNearlyZero() ? GetActorForwardVector() : Away.GetSafeNormal();
	Player->LaunchCharacter(Direction * SlamLaunchSpeed + FVector(0.f, 0.f, SlamLaunchZ), true, true);
}

void AElderTreant::SetSlamTelegraph(bool bVisible, float Progress)
{
	SlamDecal->SetVisibility(bVisible);
	if (SlamDecalMaterial)
	{
		SlamDecalMaterial->SetScalarParameterValue(SlamProgressParam, FMath::Clamp(Progress, 0.f, 1.f));
	}
}

// ---------------------------------------------------------------- Summon

void AElderTreant::SummonVines()
{
	ASpawnDirector* Director = GetSpawnDirector();
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Director || !Player || !SummonClass)
	{
		return;
	}

	// Evenly around the Treant, starting at a random angle; spots inside obstacles are skipped
	const float StartAngle = FMath::FRandRange(0.f, UE_TWO_PI);
	for (int32 i = 0; i < SummonCount; ++i)
	{
		const float Angle = StartAngle + UE_TWO_PI * i / SummonCount;
		const FVector Point = GetActorLocation() + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * SummonRadius;

		FVector Ground;
		if (FindGround(Point, true, Ground))
		{
			// Through the director, so summons count as active enemies, scale with time and drop loot
			Director->SpawnEnemy(SummonClass, Ground, Player->GetActorLocation());
		}
	}
}

// ---------------------------------------------------------------- Root Walls

void AElderTreant::RaiseRootWalls()
{
	UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Pool || !Player || !RootWallClass)
	{
		return;
	}

	const float HalfHeight = RootWallClass->GetDefaultObject<ARootWall>()->GetHalfHeight();
	const FVector Center = Player->GetActorLocation();

	// A square around the player at a random rotation. Each wall is shorter than the square's side,
	// so the corners stay open as escape gaps.
	const float BaseYaw = FMath::FRandRange(0.f, 360.f);
	for (int32 i = 0; i < RootWallCount; ++i)
	{
		const float Yaw = BaseYaw + 360.f * i / RootWallCount;
		const FVector Out = FRotator(0.f, Yaw, 0.f).Vector();

		FVector Ground;
		if (!FindGround(Center + Out * RootWallDistance, false, Ground))
		{
			continue; // off the edge of the map
		}

		// The wall's length (local X) runs across the line to the player
		const FTransform WallTransform(FRotator(0.f, Yaw + 90.f, 0.f), Ground + FVector(0.f, 0.f, HalfHeight));
		if (ARootWall* Wall = Pool->AcquireAs<ARootWall>(RootWallClass, WallTransform))
		{
			Wall->PushOutEnemies();
		}
	}
}

// ---------------------------------------------------------------- Helpers

bool AElderTreant::FindGround(const FVector& Point, bool bMustBeClear, FVector& OutGround) const
{
	// With bMustBeClear, also hit obstacles so landing on one can be rejected.
	// Without it (root walls), go straight through them to the floor.
	FCollisionObjectQueryParams Query(ECC_WorldStatic);
	if (bMustBeClear)
	{
		Query.AddObjectTypesToQuery(SUCollision::Obstacle);
	}

	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByObjectType(Hit, Point + FVector(0.f, 0.f, 1000.f),
		Point - FVector(0.f, 0.f, 2000.f), Query))
	{
		return false;
	}

	if (bMustBeClear)
	{
		const bool bOnObstacle = Hit.GetComponent() && Hit.GetComponent()->GetCollisionObjectType() == SUCollision::Obstacle;
		if (bOnObstacle || GetWorld()->OverlapAnyTestByObjectType(Hit.ImpactPoint + FVector(0.f, 0.f, SummonClearance),
			FQuat::Identity, FCollisionObjectQueryParams(SUCollision::Obstacle), FCollisionShape::MakeSphere(SummonClearance)))
		{
			return false;
		}
	}

	OutGround = Hit.ImpactPoint;
	return true;
}

ASpawnDirector* AElderTreant::GetSpawnDirector()
{
	if (!SpawnDirector.IsValid())
	{
		SpawnDirector = Cast<ASpawnDirector>(UGameplayStatics::GetActorOfClass(this, ASpawnDirector::StaticClass()));
	}
	return SpawnDirector.Get();
}