// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/BossBase.h"
#include "ElderTreant.generated.h"

class UDecalComponent;
class UMaterialInstanceDynamic;
class ASpawnDirector;
class ARootWall;

/**
 * First boss (05:00). Walks at the player slowly and cycles three attacks on their own timers:
 *  - Ground Slam: stops, telegraphs a ring, then damages and launches the player if inside it
 *  - Summon: Vine Crawlers on a ring around itself, spawned through the Spawn Director
 *  - Root Walls: four walls around the player with gaps at the corners
 * Freeze and Stun pause its attack timers as well as its movement.
 * Health, speed, contact damage, XP and knockback resistance come from its EnemyData, like any enemy.
 */
UCLASS()
class SURVIVALUNLEASHED_API AElderTreant : public ABossBase
{
	GENERATED_BODY()

public:
	AElderTreant();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual FVector GetDesiredDirection() const override;
	virtual void OnFightStarted() override;
	virtual void OnFightEnded() override;

	// ---- Ground Slam

	// Needs a deferred decal material: a ring, with an optional scalar "Progress" (0 to 1) that fills during the telegraph
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UDecalComponent> SlamDecal;

	UPROPERTY(EditDefaultsOnly, Category = "Treant|Ground Slam", meta = (ClampMin = "0.1"))
	float SlamInterval = 6.f;

	UPROPERTY(EditDefaultsOnly, Category = "Treant|Ground Slam", meta = (ClampMin = "0"))
	float SlamTelegraph = 1.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Treant|Ground Slam", meta = (ClampMin = "0"))
	float SlamRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Treant|Ground Slam", meta = (ClampMin = "0"))
	float SlamDamage = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "Treant|Ground Slam", meta = (ClampMin = "0"))
	float SlamLaunchSpeed = 1400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Treant|Ground Slam", meta = (ClampMin = "0"))
	float SlamLaunchZ = 200.f;

	// ---- Summon

	UPROPERTY(EditDefaultsOnly, Category = "Treant|Summon")
	TSubclassOf<AEnemyBase> SummonClass;

	UPROPERTY(EditDefaultsOnly, Category = "Treant|Summon", meta = (ClampMin = "0.1"))
	float SummonInterval = 15.f;

	UPROPERTY(EditDefaultsOnly, Category = "Treant|Summon", meta = (ClampMin = "1"))
	int32 SummonCount = 8;

	UPROPERTY(EditDefaultsOnly, Category = "Treant|Summon", meta = (ClampMin = "0"))
	float SummonRadius = 300.f;

	// ---- Root Walls

	UPROPERTY(EditDefaultsOnly, Category = "Treant|Root Walls")
	TSubclassOf<ARootWall> RootWallClass;

	UPROPERTY(EditDefaultsOnly, Category = "Treant|Root Walls", meta = (ClampMin = "0.1"))
	float RootWallInterval = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Treant|Root Walls", meta = (ClampMin = "1"))
	int32 RootWallCount = 4;

	// From the player to each wall's center
	UPROPERTY(EditDefaultsOnly, Category = "Treant|Root Walls", meta = (ClampMin = "0"))
	float RootWallDistance = 400.f;

private:
	float SlamCooldown = 0.f;
	float SummonCooldown = 0.f;
	float RootWallCooldown = 0.f;

	// Above zero while the slam is telegraphing; the Treant stands still until it lands
	float SlamWindupRemaining = 0.f;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SlamDecalMaterial;

	TWeakObjectPtr<ASpawnDirector> SpawnDirector;

	bool IsSlamming() const { return SlamWindupRemaining > 0.f; }

	void StartSlam();
	void ResolveSlam();
	void SetSlamTelegraph(bool bVisible, float Progress = 0.f);

	void SummonVines();
	void RaiseRootWalls();

	// Traces down for the floor at Point. With bMustBeClear, fails on top of or inside an obstacle.
	bool FindGround(const FVector& Point, bool bMustBeClear, FVector& OutGround) const;

	ASpawnDirector* GetSpawnDirector();
};