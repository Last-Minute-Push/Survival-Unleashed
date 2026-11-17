// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/BossBase.h"
#include "HiveQueen.generated.h"

class UDecalComponent;
class UMaterialInstanceDynamic;
class AHiveQueen;

// Native hooks for VFX and sounds: bind with AddUObject
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHiveQueenEvent, AHiveQueen*);

/**
 * Third boss (15:00). She never moves: she floods the arena with Hornet Swarms until the player pushes through to her.
 *  - Spawn Swarm: SwarmSize hornets around her every SwarmInterval, while fewer than SwarmCap of hers are alive
 *  - Royal Buzz: when the player is within BuzzRadius, her wings flare gold for BuzzFlareTime, then a shockwave
 *    damages and knocks back a player who's still inside
 * Freeze and Stun pause both timers. Her hornets stay and keep fighting after she dies.
 * Health, contact damage, XP, knockback resistance, move speed 0 and bIgnoresPull come from her EnemyData.
 *
 * AttackPose for the material: 0 -> 1 over the flare (wings spread and glow), 1 -> 0 after the buzz.
 */
UCLASS()
class SURVIVALUNLEASHED_API AHiveQueen : public ABossBase
{
	GENERATED_BODY()

public:
	AHiveQueen();

	virtual void Tick(float DeltaTime) override;

	// The flare starts (the telegraph), and the shockwave goes off
	FOnHiveQueenEvent OnRoyalBuzzFlare;
	FOnHiveQueenEvent OnRoyalBuzz;

	int32 GetSwarmCount() const { return Swarm.Num(); }

protected:
	virtual void BeginPlay() override;
	virtual FVector GetDesiredDirection() const override { return FVector::ZeroVector; }
	virtual void OnFightStarted() override;
	virtual void OnFightEnded() override;

	// ---- Arrival

	// She moves this far from the player when the fight starts, off screen, so the player has to go to her.
	// Shorter distances are tried if the spot is blocked or outside the play area.
	UPROPERTY(EditDefaultsOnly, Category = "Hive Queen|Arrival", meta = (ClampMin = "0"))
	float ArrivalDistance = 1800.f;

	// ---- Spawn Swarm

	UPROPERTY(EditDefaultsOnly, Category = "Hive Queen|Swarm")
	TSubclassOf<AEnemyBase> HornetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Hive Queen|Swarm", meta = (ClampMin = "0.1"))
	float SwarmInterval = 4.f;

	UPROPERTY(EditDefaultsOnly, Category = "Hive Queen|Swarm", meta = (ClampMin = "1"))
	int32 SwarmSize = 8;

	// No new hornets while this many of hers are alive
	UPROPERTY(EditDefaultsOnly, Category = "Hive Queen|Swarm", meta = (ClampMin = "1"))
	int32 SwarmCap = 60;

	// Hornets appear on a ring this far from her
	UPROPERTY(EditDefaultsOnly, Category = "Hive Queen|Swarm", meta = (ClampMin = "0"))
	float SwarmSpawnRadius = 250.f;

	// The first swarm comes this soon after the fight starts
	UPROPERTY(EditDefaultsOnly, Category = "Hive Queen|Swarm", meta = (ClampMin = "0"))
	float FirstSwarmDelay = 1.f;

	// Hornets made up front, topped up only if the pool has fewer
	UPROPERTY(EditDefaultsOnly, Category = "Hive Queen|Swarm", meta = (ClampMin = "0"))
	int32 HornetPrewarm = 40;

	// ---- Royal Buzz

	// Same kind of decal material as the Treant's slam and the Jaguar's pounce: a ring with a scalar "Progress" (0 to 1)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UDecalComponent> BuzzDecal;

	UPROPERTY(EditDefaultsOnly, Category = "Hive Queen|Royal Buzz", meta = (ClampMin = "0.1"))
	float BuzzInterval = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Hive Queen|Royal Buzz", meta = (ClampMin = "0"))
	float BuzzRadius = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Hive Queen|Royal Buzz", meta = (ClampMin = "0.01"))
	float BuzzFlareTime = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Hive Queen|Royal Buzz", meta = (ClampMin = "0"))
	float BuzzDamage = 10.f;

	// Roughly how far the player is thrown (uu), as a launch with a small hop
	UPROPERTY(EditDefaultsOnly, Category = "Hive Queen|Royal Buzz", meta = (ClampMin = "0"))
	float BuzzKnockback = 800.f;

	// The wings settle back over this long after the buzz
	UPROPERTY(EditDefaultsOnly, Category = "Hive Queen|Royal Buzz", meta = (ClampMin = "0.01"))
	float BuzzSettleTime = 0.4f;

private:
	enum class EBuzzState : uint8
	{
		Idle,
		Flare,  // telegraph: the wings spread and glow, the ring fills
		Settle  // after the shockwave: the wings fold back
	};

	EBuzzState BuzzState = EBuzzState::Idle;
	float BuzzElapsed = 0.f;
	float BuzzCooldown = 0.f;
	float SwarmCooldown = 0.f;

	// Her hornets that are still alive. Each one removes itself when it dies (HandleHornetDied).
	TArray<TWeakObjectPtr<AEnemyBase>> Swarm;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BuzzDecalMaterial;

	void MoveToArrivalSpot();
	void SpawnSwarm();
	void ForgetSwarm();
	void PruneSwarm();

	// Dynamic delegate (OnDied), so it has to be a UFUNCTION
	UFUNCTION()
	void HandleHornetDied(AEnemyBase* Hornet);

	void StartFlare();
	void ReleaseBuzz();
	void SetBuzzTelegraph(bool bVisible, float Progress = 0.f);
	// The player if they're inside the ring, otherwise null
	APawn* GetPlayerInBuzzRange() const;
	bool CanAdvanceTimers() const;
};
