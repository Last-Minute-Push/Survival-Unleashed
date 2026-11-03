// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "BossBase.generated.h"

class UBossEventSubsystem;

/**
 * Every boss reports the fight to UBossEventSubsystem,
 * drops a Boss Chest where it dies, and skips time scaling.
 * Subclasses add the attacks.
 */
UCLASS(Abstract)
class SURVIVALUNLEASHED_API ABossBase : public AEnemyBase
{
	GENERATED_BODY()

public:
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	virtual void OnPoolAcquire_Implementation() override;
	virtual void OnPoolRelease_Implementation() override;

	virtual void Tick(float DeltaTime) override;

protected:
	// Shown on the HUD boss bar
	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	FText BossName;

	// Spawned from the pool where the boss dies. Leave empty until the Boss Chest exists.
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Loot")
	TSubclassOf<AActor> BossChestClass;

	bool IsFightActive() const { return bFightActive; }

	// Reset attack timers here: pooled bosses are reused
	virtual void OnFightStarted() {}

	// Cancel anything in progress (telegraphs, casts)
	virtual void OnFightEnded() {}

private:
	void StartFight();
	void EndFight(bool bDied);
	void SpawnBossChest() const;
	UBossEventSubsystem* GetBossEvents() const;

	// Set on acquire, started on the first Tick, after the Spawn Director has finished setting us up
	bool bFightPending = false;
	bool bFightActive = false;

	// Updated before anything that can kill us
	FVector LastLocation = FVector::ZeroVector;
};