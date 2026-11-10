// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/JungleCat.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void AJungleCat::OnPoolAcquire_Implementation()
{
	Super::OnPoolAcquire_Implementation();

	// Pooled cats start as normal chasers; the Jaguar calls JoinPack right after spawning one
	PackState = EPackState::None;
	LungeTravelled = 0.f;
	SetMaterialData(EnemyMaterialData::AttackPose, 0.f);
}

void AJungleCat::JoinPack(float StartAngle, float LungeTime)
{
	// Every cat in a pack turns at the same rate, so storing the angle at time 0 keeps them evenly spaced
	const float AngularSpeed = CircleRadius > 0.f ? CircleSpeed / CircleRadius : 0.f;
	SlotAngleAtZero = StartAngle - GetWorld()->GetTimeSeconds() * AngularSpeed;
	PackLungeTime = LungeTime;
	PackState = EPackState::Circling;
}

void AJungleCat::LeavePack()
{
	if (PackState == EPackState::Circling)
	{
		PackState = EPackState::None;
		SetMaterialData(EnemyMaterialData::AttackPose, 0.f);
	}
}

void AJungleCat::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead())
	{
		return;
	}

	if (PackState == EPackState::Lunging)
	{
		UpdateLunge(DeltaTime);
		return;
	}

	if (PackState != EPackState::Circling)
	{
		return;
	}

	const bool bDisabled = IsStatusActive(EEnemyStatus::Freeze) || IsStatusActive(EEnemyStatus::Stun);
	const float Now = GetWorld()->GetTimeSeconds();

	if (Now >= PackLungeTime)
	{
		// The pack's moment has passed: a cat that's frozen or stunned misses it
		if (bDisabled)
		{
			LeavePack();
		}
		else
		{
			StartLunge();
		}
		return;
	}

	// Crouch over the last LungeCrouchTime, facing the player
	if (IsCrouching())
	{
		const float Alpha = LungeCrouchTime > 0.f ? 1.f - (PackLungeTime - Now) / LungeCrouchTime : 1.f;
		SetMaterialData(EnemyMaterialData::AttackPose, 0.5f * FMath::Clamp(Alpha, 0.f, 1.f));

		if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			const FVector ToPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
			if (!ToPlayer.IsNearlyZero())
			{
				SetActorRotation(FMath::RInterpTo(GetActorRotation(), FRotator(0.f, ToPlayer.Rotation().Yaw, 0.f), DeltaTime, TurnSpeed));
			}
		}
	}
}

FVector AJungleCat::GetDesiredDirection() const
{
	if (PackState == EPackState::Lunging || IsCrouching())
	{
		return FVector::ZeroVector; // the lunge moves itself; the crouch holds still
	}
	if (PackState != EPackState::Circling)
	{
		return Super::GetDesiredDirection();
	}

	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return FVector::ZeroVector;
	}

	// Toward its place on the turning circle: full speed when far, easing off as it arrives so it stays on it
	FVector ToSlot = GetSlotPoint(Player->GetActorLocation()) - GetActorLocation();
	ToSlot.Z = 0.f;
	const FVector Desired = (ToSlot / SlotSlowRadius).GetClampedToMaxSize2D(1.f);

	// A little separation, so other enemies don't stack on the cats' places
	return (Desired + ComputeSeparation() * SeparationWeight * 0.5f).GetClampedToMaxSize2D(1.f);
}

bool AJungleCat::IsCrouching() const
{
	return PackState == EPackState::Circling && GetWorld()->GetTimeSeconds() >= PackLungeTime - LungeCrouchTime;
}

FVector AJungleCat::GetSlotPoint(const FVector& PlayerLocation) const
{
	const float AngularSpeed = CircleRadius > 0.f ? CircleSpeed / CircleRadius : 0.f;
	const float Angle = SlotAngleAtZero + GetWorld()->GetTimeSeconds() * AngularSpeed;
	return PlayerLocation + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * CircleRadius;
}

void AJungleCat::StartLunge()
{
	// Locked at the start: at the player's position right now, so stepping aside dodges it
	FVector Direction = GetActorForwardVector().GetSafeNormal2D();
	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		const FVector ToPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		if (!ToPlayer.IsNearlyZero())
		{
			Direction = ToPlayer;
		}
	}

	PackState = EPackState::Lunging;
	LungeDirection = Direction;
	LungeTravelled = 0.f;
	SetActorRotation(FRotator(0.f, Direction.Rotation().Yaw, 0.f));
	SetMaterialData(EnemyMaterialData::AttackPose, 1.f);
}

void AJungleCat::UpdateLunge(float DeltaTime)
{
	// Frozen or stunned mid-lunge: it stops where it is
	if (IsStatusActive(EEnemyStatus::Freeze) || IsStatusActive(EEnemyStatus::Stun))
	{
		EndLunge();
		return;
	}

	const float Step = FMath::Min(LungeSpeed * DeltaTime, LungeDistance - LungeTravelled);

	// Swept, so it stops at obstacles. Its contact damage does the hitting as it runs into the player.
	FHitResult Hit;
	AddActorWorldOffset(LungeDirection * Step, true, &Hit);
	LungeTravelled += Step;

	if (Hit.bBlockingHit || LungeTravelled >= LungeDistance - KINDA_SMALL_NUMBER)
	{
		EndLunge();
	}
}

void AJungleCat::EndLunge()
{
	// Back to a normal chase
	PackState = EPackState::None;
	SetMaterialData(EnemyMaterialData::AttackPose, 0.f);
}
