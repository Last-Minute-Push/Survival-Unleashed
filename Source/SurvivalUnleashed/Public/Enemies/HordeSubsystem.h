// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HordeSubsystem.generated.h"

/**
 *
 */

class AEnemyBase;

UCLASS()
class SURVIVALUNLEASHED_API UHordeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterEnemy(AEnemyBase* Enemy);
	void UnregisterEnemy(AEnemyBase* Enemy);

	// Active enemies within Radius of Location (2D). May include the caller.
	void GetNeighbors(const FVector& Location, float Radius, TArray<AEnemyBase*>& OutNeighbors) const;

	// ---- Weapon targeting: only living, active enemies

	// Closest enemy within MaxRange (2D), or nullptr
	AEnemyBase* FindNearestEnemy(const FVector& Location, float MaxRange) const;

	// Every enemy within Radius (2D), in no particular order
	void GetEnemiesInRadius(const FVector& Location, float Radius, TArray<AEnemyBase*>& OutEnemies) const;

	// Up to Count enemies within MaxRange, closest first
	void GetNearestEnemies(const FVector& Location, float MaxRange, int32 Count, TArray<AEnemyBase*>& OutEnemies) const;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	static constexpr float CellSize = 200.f;

	UPROPERTY()
	TArray<AEnemyBase*> Enemies;

	TMap<FIntPoint, TArray<AEnemyBase*>> Grid;

	static FIntPoint ToCell(const FVector& Location);

	void DrawDebugGrid() const;

	// The grid is rebuilt once per frame, so an enemy killed earlier this frame is still in it
	static bool IsTargetable(const AEnemyBase * Enemy);
};
