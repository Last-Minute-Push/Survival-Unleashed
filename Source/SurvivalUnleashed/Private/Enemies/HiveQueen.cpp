// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/HiveQueen.h"
#include "Enemies/SpawnDirector.h"
#include "Framework/ObjectPoolSubsystem.h"
#include "Utility/PlayerStatsComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

namespace
{
	const FName BuzzProgressParam(TEXT("Progress"));
}

AHiveQueen::AHiveQueen()
{
	// She never moves or turns, so the ring can stay attached to her
	BuzzDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("BuzzDecal"));
	BuzzDecal->SetupAttachment(GetRootComponent());
	BuzzDecal->SetUsingAbsoluteRotation(true);
	BuzzDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f)); // project straight down
	BuzzDecal->SetFadeScreenSize(0.f);
	BuzzDecal->SetVisibility(false);
}

void AHiveQueen::BeginPlay()
{
	Super::BeginPlay();

	// Sized here so a Blueprint's BuzzRadius is used. X is the projection depth.
	BuzzDecal->DecalSize = FVector(200.f, BuzzRadius, BuzzRadius);
	BuzzDecal->MarkRenderStateDirty();
	BuzzDecalMaterial = BuzzDecal->CreateDynamicMaterialInstance();

	// The timeline's Hornet events already prewarm some; top the pool up so her swarms don't spawn new actors mid-fight
	UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (Pool && HornetClass)
	{
		const int32 Missing = HornetPrewarm - Pool->GetInactiveCount(HornetClass);
		if (Missing > 0)
		{
			Pool->Prewarm(HornetClass, Missing);
		}
	}
}

void AHiveQueen::OnFightStarted()
{
	BuzzState = EBuzzState::Idle;
	BuzzElapsed = 0.f;
	BuzzCooldown = BuzzInterval;
	SwarmCooldown = FirstSwarmDelay;
	ForgetSwarm();

	SetBuzzTelegraph(false);
	SetMaterialData(EnemyMaterialData::AttackPose, 0.f);

	MoveToArrivalSpot();
}

void AHiveQueen::OnFightEnded()
{
	SetBuzzTelegraph(false);
	SetMaterialData(EnemyMaterialData::AttackPose, 0.f);

	// Her hornets keep fighting as normal enemies; she just stops counting them
	ForgetSwarm();
}

void AHiveQueen::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsFightActive() || IsDead() || !CanAdvanceTimers())
	{
		return;
	}

	// ---- Spawn Swarm
	SwarmCooldown -= DeltaTime;
	if (SwarmCooldown <= 0.f)
	{
		SwarmCooldown = SwarmInterval;
		SpawnSwarm();
	}

	// ---- Royal Buzz
	BuzzElapsed += DeltaTime;
	switch (BuzzState)
	{
	case EBuzzState::Idle:
	{
		BuzzCooldown -= DeltaTime;
		if (BuzzCooldown <= 0.f && GetPlayerInBuzzRange())
		{
			StartFlare();
		}
		break;
	}

	case EBuzzState::Flare:
	{
		const float Alpha = FMath::Clamp(BuzzElapsed / BuzzFlareTime, 0.f, 1.f);
		SetMaterialData(EnemyMaterialData::AttackPose, Alpha);
		SetBuzzTelegraph(true, Alpha);
		if (Alpha >= 1.f)
		{
			ReleaseBuzz();
		}
		break;
	}

	case EBuzzState::Settle:
	{
		const float Alpha = FMath::Clamp(BuzzElapsed / BuzzSettleTime, 0.f, 1.f);
		SetMaterialData(EnemyMaterialData::AttackPose, 1.f - FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.f));
		if (Alpha >= 1.f)
		{
			BuzzState = EBuzzState::Idle;
		}
		break;
	}
	}
}

// ---------------------------------------------------------------- Arrival

void AHiveQueen::MoveToArrivalSpot()
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(GetRootComponent());
	if (!Player || !CapsuleComp)
	{
		return;
	}

	// Keep the side the Spawn Director picked, and turn away from it only as far as needed to find clear ground
	const FVector PlayerLocation = Player->GetActorLocation();
	FVector Away = (GetActorLocation() - PlayerLocation).GetSafeNormal2D();
	if (Away.IsNearlyZero())
	{
		Away = FVector::ForwardVector;
	}
	const float BaseAngle = FMath::Atan2(Away.Y, Away.X);
	const float Radius = CapsuleComp->GetScaledCapsuleRadius();
	const float HalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();

	for (const float DistanceScale : { 1.f, 0.8f, 0.6f })
	{
		for (int32 Step = 0; Step <= 6; ++Step)
		{
			// 0, +30, -30, +60, -60, ... degrees
			const float Offset = FMath::DegreesToRadians(30.f * ((Step + 1) / 2)) * (Step % 2 == 0 ? -1.f : 1.f);
			const float Angle = BaseAngle + Offset;
			const FVector Point = PlayerLocation + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * ArrivalDistance * DistanceScale;

			FVector Ground;
			if (FindGround(Point, true, Ground, Radius + 20.f))
			{
				SetActorLocation(Ground + FVector(0.f, 0.f, HalfHeight));
				return;
			}
		}
	}
	// Nowhere clear: stay where the Spawn Director put her
}

// ---------------------------------------------------------------- Spawn Swarm

void AHiveQueen::SpawnSwarm()
{
	ASpawnDirector* Director = GetSpawnDirector();
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Director || !Player || !HornetClass)
	{
		return;
	}

	PruneSwarm();
	const int32 Count = FMath::Min(SwarmSize, SwarmCap - Swarm.Num());
	if (Count <= 0)
	{
		return;
	}

	const FVector PlayerLocation = Player->GetActorLocation();
	const float StartAngle = FMath::FRandRange(0.f, UE_TWO_PI);
	for (int32 i = 0; i < Count; ++i)
	{
		const float Angle = StartAngle + UE_TWO_PI * i / Count;
		const FVector Point = GetActorLocation() + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * SwarmSpawnRadius;

		FVector Ground;
		if (!FindGround(Point, true, Ground))
		{
			continue;
		}

		// Through the director, so hornets count as active enemies, scale with time and drop loot
		if (AEnemyBase* Hornet = Director->SpawnEnemy(HornetClass, Ground, PlayerLocation))
		{
			Hornet->OnDied.AddUniqueDynamic(this, &AHiveQueen::HandleHornetDied);
			Swarm.Add(Hornet);
		}
	}
}

void AHiveQueen::HandleHornetDied(AEnemyBase* Hornet)
{
	// Unbound, so this pooled hornet doesn't report to her again in a later life as an ordinary enemy
	if (Hornet)
	{
		Hornet->OnDied.RemoveDynamic(this, &AHiveQueen::HandleHornetDied);
	}
	Swarm.Remove(Hornet);
}

void AHiveQueen::PruneSwarm()
{
	// A hornet put back in the pool without dying (hidden) no longer counts toward the cap
	Swarm.RemoveAll([this](const TWeakObjectPtr<AEnemyBase>& Weak)
	{
		AEnemyBase* Hornet = Weak.Get();
		if (!Hornet || Hornet->IsDead() || Hornet->IsHidden())
		{
			if (Hornet)
			{
				Hornet->OnDied.RemoveDynamic(this, &AHiveQueen::HandleHornetDied);
			}
			return true;
		}
		return false;
	});
}

void AHiveQueen::ForgetSwarm()
{
	for (const TWeakObjectPtr<AEnemyBase>& Weak : Swarm)
	{
		if (AEnemyBase* Hornet = Weak.Get())
		{
			Hornet->OnDied.RemoveDynamic(this, &AHiveQueen::HandleHornetDied);
		}
	}
	Swarm.Reset();
}

// ---------------------------------------------------------------- Royal Buzz

void AHiveQueen::StartFlare()
{
	BuzzState = EBuzzState::Flare;
	BuzzElapsed = 0.f;
	SetBuzzTelegraph(true, 0.f);
	OnRoyalBuzzFlare.Broadcast(this);
}

void AHiveQueen::ReleaseBuzz()
{
	BuzzState = EBuzzState::Settle;
	BuzzElapsed = 0.f;
	BuzzCooldown = BuzzInterval;
	SetBuzzTelegraph(false);
	OnRoyalBuzz.Broadcast(this);

	// Only a player who stayed inside the ring through the flare gets hit
	APawn* Target = GetPlayerInBuzzRange();
	if (!Target)
	{
		return;
	}

	// Damage first: a dash or Evasion can block it, and a blocked hit doesn't knock them back either.
	// The player's damage handler doesn't report a dodge, so compare health before and after (as the Root Worm does).
	UPlayerStatsComponent* Stats = Target->FindComponentByClass<UPlayerStatsComponent>();
	const float HealthBefore = Stats ? Stats->GetCurrentHealth() : 0.f;
	UGameplayStatics::ApplyDamage(Target, BuzzDamage, nullptr, this, nullptr);
	const bool bLanded = !Stats || Stats->GetCurrentHealth() < HealthBefore;

	ACharacter* PlayerCharacter = Cast<ACharacter>(Target);
	if (!bLanded || !PlayerCharacter || BuzzKnockback <= 0.f)
	{
		return;
	}

	FVector Away = (PlayerCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (Away.IsNearlyZero())
	{
		Away = FVector::ForwardVector;
	}

	// Launched with a hop of 0.3 x the horizontal speed: friction stops a grounded launch almost at once.
	// Air time = 2 * 0.3v / g, so distance = 0.6 v^2 / g, and v = sqrt(distance * g / 0.6).
	const float Gravity = FMath::Max(1.f, FMath::Abs(PlayerCharacter->GetCharacterMovement()->GetGravityZ()));
	const float Speed = FMath::Sqrt(BuzzKnockback * Gravity / 0.6f);
	PlayerCharacter->LaunchCharacter(Away * Speed + FVector(0.f, 0.f, Speed * 0.3f), true, true);
}

APawn* AHiveQueen::GetPlayerInBuzzRange() const
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	return Player && FVector::Dist2D(Player->GetActorLocation(), GetActorLocation()) <= BuzzRadius ? Player : nullptr;
}

void AHiveQueen::SetBuzzTelegraph(bool bVisible, float Progress)
{
	BuzzDecal->SetVisibility(bVisible);
	if (BuzzDecalMaterial)
	{
		BuzzDecalMaterial->SetScalarParameterValue(BuzzProgressParam, FMath::Clamp(Progress, 0.f, 1.f));
	}
}

bool AHiveQueen::CanAdvanceTimers() const
{
	return !IsStatusActive(EEnemyStatus::Freeze) && !IsStatusActive(EEnemyStatus::Stun);
}
