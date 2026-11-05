// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/ChargingBoar.h"
#include "Framework/SUCollision.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

namespace
{
	const FName PathProgressParam(TEXT("Progress"));
}

AChargingBoar::AChargingBoar()
{
	PathDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("PathDecal"));
	PathDecal->SetupAttachment(GetRootComponent());
	PathDecal->SetUsingAbsoluteLocation(true); // stays on the ground where it was drawn, even if the boar is nudged
	PathDecal->SetUsingAbsoluteRotation(true);
	PathDecal->SetVisibility(false);
}

void AChargingBoar::BeginPlay()
{
	Super::BeginPlay();
	PathDecalMaterial = PathDecal->CreateDynamicMaterialInstance();
}

void AChargingBoar::OnPoolAcquire_Implementation()
{
	Super::OnPoolAcquire_Implementation();
	CooldownRemaining = 1.f; // a moment on the field before the first charge
	SetState(EState::Approach);
}

float AChargingBoar::GetContactDamage() const
{
	// While charging, the ram (one hit of ChargeDamage) replaces contact damage
	return State == EState::Charge ? 0.f : Super::GetContactDamage();
}

FVector AChargingBoar::GetDesiredDirection() const
{
	// Only walks while approaching; the charge moves itself, and the wind-up and stun stand still
	return State == EState::Approach ? Super::GetDesiredDirection() : FVector::ZeroVector;
}

void AChargingBoar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (IsDead() || !Player)
	{
		return;
	}

	// Frozen or stunned by a weapon: a wind-up waits, a charge stops where it is
	const bool bDisabled = IsStatusActive(EEnemyStatus::Freeze) || IsStatusActive(EEnemyStatus::Stun);

	StateTime += DeltaTime;

	switch (State)
	{
	case EState::Approach:
		CooldownRemaining -= DeltaTime;
		if (!bDisabled && CanStartCharge(Player))
		{
			StartWindup(Player);
		}
		break;

	case EState::Windup:
		if (bDisabled)
		{
			StateTime -= DeltaTime;
		}
		else if (StateTime >= WindupTime)
		{
			HidePath();
			ChargeRemaining = ChargeLength;
			bRammedThisCharge = false;
			SetState(EState::Charge);
		}
		else if (PathDecalMaterial)
		{
			PathDecalMaterial->SetScalarParameterValue(PathProgressParam, StateTime / WindupTime);
		}
		break;

	case EState::Charge:
		if (bDisabled)
		{
			CooldownRemaining = ChargeCooldown;
			SetState(EState::Approach);
		}
		else
		{
			TickCharge(Player, DeltaTime);
		}
		break;

	case EState::Stunned:
		if (StateTime >= StunTime)
		{
			CooldownRemaining = ChargeCooldown;
			SetState(EState::Approach);
		}
		break;
	}

	// Material pose: head lowering and eyes lighting up through the wind-up, held through the charge
	const float Pose = State == EState::Windup ? FMath::Clamp(StateTime / WindupTime, 0.f, 1.f)
		: State == EState::Charge ? 1.f : 0.f;
	SetMaterialData(EnemyMaterialData::AttackPose, Pose);
}

bool AChargingBoar::CanStartCharge(const APawn* Player) const
{
	if (CooldownRemaining > 0.f)
	{
		return false;
	}

	const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	if (ToPlayer.SizeSquared2D() > FMath::Square(ChargeTriggerRange))
	{
		return false;
	}

	// Clear line: nothing solid between it and the player (it wouldn't charge into a pillar on purpose)
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BoarSightLine), false, this);
	return !GetWorld()->LineTraceTestByObjectType(GetActorLocation(), Player->GetActorLocation(),
		FCollisionObjectQueryParams(SUCollision::Obstacle), Params);
}

void AChargingBoar::StartWindup(const APawn* Player)
{
	// Locked now: the player dodges by leaving the line during the wind-up
	FVector Direction = Player->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.f;
	ChargeDirection = Direction.GetSafeNormal();
	if (ChargeDirection.IsNearlyZero())
	{
		ChargeDirection = GetActorForwardVector();
	}

	SetActorRotation(ChargeDirection.Rotation());
	ShowPath(MeasureChargePath(ChargeDirection));
	SetState(EState::Windup);
}

void AChargingBoar::TickCharge(APawn* Player, float DeltaTime)
{
	const float Step = FMath::Min(ChargeSpeed * DeltaTime, ChargeRemaining);

	// Swept, so it stops at the first obstacle instead of tunneling through at this speed.
	// The Enemy profile overlaps the player and ignores other enemies, so only solid things stop it.
	FHitResult Hit;
	AddActorWorldOffset(ChargeDirection * Step, true, &Hit);
	ChargeRemaining -= Step;

	// Ram: one hit per charge on the first touch
	if (!bRammedThisCharge)
	{
		const UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(GetRootComponent());
		const float Reach = (CapsuleComp ? CapsuleComp->GetScaledCapsuleRadius() : 50.f) + Player->GetSimpleCollisionRadius();
		if (FVector::DistSquared2D(GetActorLocation(), Player->GetActorLocation()) <= FMath::Square(Reach))
		{
			bRammedThisCharge = true;
			UGameplayStatics::ApplyDamage(Player, ChargeDamage, nullptr, this, nullptr);
		}
	}

	SetMaterialData(EnemyMaterialData::MoveAmount, ChargeMoveAmount);

	if (Hit.bBlockingHit)
	{
		// Pillars, logs, root walls and the jungle wall all use the Obstacle profile
		const bool bHitObstacle = Hit.GetComponent() && Hit.GetComponent()->GetCollisionObjectType() == SUCollision::Obstacle;
		if (bHitObstacle)
		{
			ApplyStatus(EEnemyStatus::Stun, StunTime, 0.f);
			SetState(EState::Stunned);
			return;
		}
		ChargeRemaining = 0.f; // anything else solid just ends the charge
	}

	if (ChargeRemaining <= KINDA_SMALL_NUMBER)
	{
		CooldownRemaining = ChargeCooldown;
		SetState(EState::Approach);
	}
}

float AChargingBoar::MeasureChargePath(const FVector& Direction) const
{
	// The same capsule sweep the charge will make, against obstacles only
	const UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(GetRootComponent());
	if (!CapsuleComp)
	{
		return ChargeLength;
	}

	const FVector Start = GetActorLocation();
	const FVector End = Start + Direction * ChargeLength;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BoarChargePath), false, this);
	FHitResult Hit;
	if (GetWorld()->SweepSingleByObjectType(Hit, Start, End, GetActorQuat(),
		FCollisionObjectQueryParams(SUCollision::Obstacle), Capsule->GetCollisionShape(), Params))
	{
		return Hit.Distance;
	}
	return ChargeLength;
}

void AChargingBoar::ShowPath(float Length)
{
	const UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(GetRootComponent());
	const float Radius = CapsuleComp ? CapsuleComp->GetScaledCapsuleRadius() : 50.f;
	const float HalfHeight = CapsuleComp ? CapsuleComp->GetScaledCapsuleHalfHeight() : 50.f;

	// The strip covers the whole swept area: from the boar's back to its front at the stopping point
	const float StripLength = Length + Radius * 2.f;
	const FVector Center = GetActorLocation() + ChargeDirection * (Length * 0.5f);

	// Pitch -90 projects straight down; the decal's local Z then runs along the yaw (the charge direction)
	// and its local Y across it. DecalSize is a half size: (depth, half width, half length).
	PathDecal->SetWorldLocationAndRotation(Center, FRotator(-90.f, ChargeDirection.Rotation().Yaw, 0.f));
	PathDecal->DecalSize = FVector(HalfHeight + 100.f, Radius, StripLength * 0.5f);
	PathDecal->MarkRenderStateDirty();

	if (PathDecalMaterial)
	{
		PathDecalMaterial->SetScalarParameterValue(PathProgressParam, 0.f);
	}
	PathDecal->SetVisibility(true);
}

void AChargingBoar::HidePath()
{
	PathDecal->SetVisibility(false);
}

void AChargingBoar::SetState(EState NewState)
{
	const bool bWasCharging = State == EState::Charge;
	State = NewState;
	StateTime = 0.f;

	if (State != EState::Windup)
	{
		HidePath();
	}

	// Stop the legs running once the charge is over; the base class takes MoveAmount back from here
	if (bWasCharging && State != EState::Charge)
	{
		SetMaterialData(EnemyMaterialData::MoveAmount, 0.f);
	}
}
