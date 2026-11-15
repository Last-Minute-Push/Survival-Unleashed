// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlowField.generated.h"

class UBoxComponent;

/**
 * A flow field toward the player over the whole play area, so the horde walks around obstacles instead of into them.
 * Place one in the level and size its box to cover the ground inside the jungle wall, with the box's bottom on the floor.
 *
 * Three layers on one grid of CellSize squares:
 *  - Blocked:  which cells an obstacle covers. Tested once at BeginPlay (and again for an area with RefreshArea).
 *  - Distance: walking distance from each cell to the player's cell (Dijkstra). Redone only when the player
 *              moves into a different cell, so it costs nothing while the player stands still.
 *  - Flow:     from each cell, the direction to its neighbor that is closest to the player.
 *
 * Enemies ask GetSeekDirection. Where nothing on the grid blocks the straight line to the player it just returns that
 * line, so in the open the horde moves exactly as before and only bends where an obstacle is actually in the way.
 */
UCLASS()
class SURVIVALUNLEASHED_API AFlowField : public AActor
{
	GENERATED_BODY()

public:
	AFlowField();

	virtual void Tick(float DeltaTime) override;

	// Unit 2D direction an enemy standing at From should walk to reach the player at PlayerLocation
	FVector GetSeekDirection(const FVector& From, const FVector& PlayerLocation) const;

	// Re-tests the cells inside Area (for example, a root wall going up or coming down) and rebuilds the field
	void RefreshArea(const FBox& Area);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> Bounds;

	UPROPERTY(EditAnywhere, Category = "Flow Field", meta = (ClampMin = "25"))
	float CellSize = 100.f;

	// Room kept from obstacles (about a typical enemy's radius), so the field doesn't lead enemies into corners
	UPROPERTY(EditAnywhere, Category = "Flow Field", meta = (ClampMin = "0"))
	float Clearance = 30.f;

	// Height above the floor tested for obstacles; low enough to catch a fallen log
	UPROPERTY(EditAnywhere, Category = "Flow Field", meta = (ClampMin = "1"))
	float ProbeHeight = 100.f;

	// The field is redone at most this often, and only after the player has moved to another cell
	UPROPERTY(EditAnywhere, Category = "Flow Field", meta = (ClampMin = "0"))
	float MinRebuildInterval = 0.1f;

	// With su.DebugAI on: arrows for this many cells around the player, blocked cells in red
	UPROPERTY(EditAnywhere, Category = "Flow Field|Debug", meta = (ClampMin = "0"))
	int32 DebugDrawRadius = 12;

private:
	FVector2D Origin = FVector2D::ZeroVector; // world XY of the grid's minimum corner
	float FloorZ = 0.f;
	int32 Width = 0;
	int32 Height = 0;

	TArray<uint8> Blocked;      // 1 = an obstacle covers this cell
	TArray<float> Distance;     // walking distance to the goal cell; MAX_flt = can't get there
	TArray<FVector2f> Flow;     // unit direction toward the goal; zero for blocked or cut-off cells

	// Per cell: -1 not checked yet, 0 something blocks the straight line to the goal, 1 clear.
	// Filled in as enemies ask, and reset on every rebuild. Mutable: it's a cache, not state.
	mutable TArray<int8> LineOfSight;

	FIntPoint Goal = FIntPoint(-1, -1); // the player's cell when the field was last built
	float TimeSinceRebuild = 0.f;
	bool bForceRebuild = false;

	bool IsValidCell(int32 X, int32 Y) const { return X >= 0 && Y >= 0 && X < Width && Y < Height; }
	int32 ToIndex(int32 X, int32 Y) const { return Y * Width + X; }
	FIntPoint ToCell(const FVector& Location) const;
	FVector2D GetCellCenter(int32 X, int32 Y) const;

	bool TestBlocked(int32 X, int32 Y) const;

	// A diagonal step is only allowed when both cells beside it are open, so paths never cut an obstacle's corner
	bool CanStep(int32 X, int32 Y, int32 DX, int32 DY) const;

	void Rebuild(const FIntPoint& NewGoal);
	bool HasLineOfSight(const FIntPoint& From) const;
	void DrawDebug() const;
};
