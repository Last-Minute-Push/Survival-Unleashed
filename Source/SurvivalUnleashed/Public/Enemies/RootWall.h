// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Framework/Poolable.h"
#include "RootWall.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/**
 * A wall of roots the Elder Treant raises around the player. Uses the Obstacle profile, so it blocks
 * the player, enemies and projectiles, and the Spawn Director won't spawn inside it.
 * Pooled; goes back to the pool after Lifetime.
 * Local X is the wall's length and local Y its thickness. The actor's origin is the box center.
 */
UCLASS()
class SURVIVALUNLEASHED_API ARootWall : public AActor, public IPoolable
{
	GENERATED_BODY()

public:
	ARootWall();

	virtual void Tick(float DeltaTime) override;
	virtual void OnPoolAcquire_Implementation() override;
	virtual void OnPoolRelease_Implementation() override;

	// Moves any enemy caught inside the wall out to its nearer side. Call right after placing the wall.
	void PushOutEnemies() const;

	// Works on the class default object too, for placing the wall before acquiring it
	float GetHalfHeight() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, Category = "Root Wall", meta = (ClampMin = "0.1"))
	float Lifetime = 5.f;

	// Seconds for the roots to burst out of the ground, and to sink back before the wall expires.
	// Only the mesh moves: the wall blocks from the moment it appears until it expires.
	UPROPERTY(EditDefaultsOnly, Category = "Root Wall", meta = (ClampMin = "0.01"))
	float RiseTime = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Root Wall", meta = (ClampMin = "0.01"))
	float SinkTime = 0.3f;

private:
	FTimerHandle LifetimeTimer;

	float Age = 0.f;
	float MeshRestZ = 0.f; // the mesh's relative Z as placed in the Blueprint

	// Lowers the mesh below the floor by how far it has risen or sunk
	void UpdateMeshHeight();

	void Expire();
};