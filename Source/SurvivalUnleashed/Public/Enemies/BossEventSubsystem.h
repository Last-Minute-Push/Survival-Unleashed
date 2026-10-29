// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BossEventSubsystem.generated.h"

// Fires once when a boss fight starts
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBossSpawned, const FText& /*BossName*/, float /*MaxHealth*/);

// Fires every time the boss's health changes (damage or healing)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBossHealthChanged, float /*CurrentHealth*/, float /*MaxHealth*/);

// Fires once when the boss dies
DECLARE_MULTICAST_DELEGATE(FOnBossDied);

/**
 * One place for boss fight events, so listeners (HUD, Spawn Director) can bind before any boss exists.
 * Bosses report in and listeners never need a pointer to the boss. One boss at a time.
 *
 * Debug (non-shipping builds), for testing listeners without a real boss:
 *   su.Boss.FakeSpawn [MaxHealth] [Name]   e.g. su.Boss.FakeSpawn 3000 Elder Treant
 *   su.Boss.FakeDamage <Amount>            kills the fake boss when it reaches 0
 *   su.Boss.FakeDie
 */
UCLASS()
class SURVIVALUNLEASHED_API UBossEventSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	FOnBossSpawned OnBossSpawned;
	FOnBossHealthChanged OnBossHealthChanged;
	FOnBossDied OnBossDied;

	// Called by a boss when its fight starts. Starts at full health. Replaces any boss still active.
	void ReportBossSpawned(AActor* Boss, const FText& BossName, float MaxHealth);

	// Called by the boss after its health changes. Ignored when no boss is active.
	void ReportBossHealth(float CurrentHealth);

	// Called by the boss when it dies. Ignored when no boss is active, so a double death can't fire twice.
	void ReportBossDied();

	// For listeners created mid-fight (a HUD rebuilt after a resolution change, for example)
	bool IsBossActive() const { return bBossActive; }
	const FText& GetBossName() const { return BossName; }
	float GetBossHealth() const { return BossHealth; }
	float GetBossMaxHealth() const { return BossMaxHealth; }

	// Null during a fake fight from the debug commands
	AActor* GetBoss() const { return Boss.Get(); }

protected:
	// Only real game worlds, not editor preview worlds
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	TWeakObjectPtr<AActor> Boss;
	FText BossName;
	float BossHealth = 0.f;
	float BossMaxHealth = 0.f;
	bool bBossActive = false;
};
