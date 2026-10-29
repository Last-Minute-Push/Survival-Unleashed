// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/BossBase.h"
#include "Enemies/BossEventSubsystem.h"
#include "Framework/ObjectPoolSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

float ABossBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bFightPending)
	{
		StartFight();
	}

	if (bFightActive)
	{
		LastLocation = GetActorLocation();
	}

	const float Result = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// If dying released us to the pool, OnPoolRelease has already ended the fight
	if (bFightActive)
	{
		if (UBossEventSubsystem* Events = GetBossEvents())
		{
			Events->ReportBossHealth(GetCurrentHealth());
		}
		if (IsDead())
		{
			EndFight(true);
		}
	}
	return Result;
}

void ABossBase::OnPoolAcquire_Implementation()
{
	Super::OnPoolAcquire_Implementation();
	bFightPending = true;
	bFightActive = false;
}

void ABossBase::OnPoolRelease_Implementation()
{
	if (bFightActive)
	{
		EndFight(IsDead());
	}
	bFightPending = false;

	Super::OnPoolRelease_Implementation();
}

void ABossBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bFightPending && !IsDead())
	{
		StartFight();
	}
	if (bFightActive)
	{
		LastLocation = GetActorLocation();
	}
}

void ABossBase::StartFight()
{
	bFightPending = false;
	bFightActive = true;
	LastLocation = GetActorLocation();

	OnFightStarted();

	if (UBossEventSubsystem* Events = GetBossEvents())
	{
		// Bosses skip time scaling, so current health is the full health
		Events->ReportBossSpawned(this, BossName, GetCurrentHealth());
	}
}

void ABossBase::EndFight(bool bDied)
{
	// Cleared first so a release during this call can't end the fight twice
	bFightActive = false;

	OnFightEnded();

	if (bDied)
	{
		SpawnBossChest();
	}

	// Also sent when the boss leaves without dying, so the HUD bar never sticks
	if (UBossEventSubsystem* Events = GetBossEvents())
	{
		Events->ReportBossDied();
	}
}

void ABossBase::SpawnBossChest() const
{
	UObjectPoolSubsystem* Pool = GetWorld() ? GetWorld()->GetSubsystem<UObjectPoolSubsystem>() : nullptr;
	if (!BossChestClass || !Pool)
	{
		return;
	}

	// LastLocation is the capsule's center, so drop it to the ground
	const UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(GetRootComponent());
	const float HalfHeight = CapsuleComp ? CapsuleComp->GetScaledCapsuleHalfHeight() : 0.f;
	Pool->Acquire(BossChestClass, FTransform(LastLocation - FVector(0.f, 0.f, HalfHeight)));
}

UBossEventSubsystem* ABossBase::GetBossEvents() const
{
	return GetWorld() ? GetWorld()->GetSubsystem<UBossEventSubsystem>() : nullptr;
}