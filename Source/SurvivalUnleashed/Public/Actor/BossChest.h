// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Framework/Poolable.h"
#include "BossChest.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * The reward for beating a boss. Every boss spawns one from the pool where it dies (BossBase's BossChestClass).
 * The player collects it by walking into it, which gives three level-up menus in a row ("BOSS CHEST 1/3" to "3/3").
 * It never expires, and gem magnets don't pull it.
 */
UCLASS()
class SURVIVALUNLEASHED_API ABossChest : public AActor, public IPoolable
{
	GENERATED_BODY()

public:
	ABossChest();

	virtual void OnPoolAcquire_Implementation() override;

protected:
	// The pickup area: the player collects the chest when their body enters it. 120 uu radius by default.
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> PickupSphere;

	// Visual only, no collision. Origin on the ground. (SM_BossChest)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// The light pillar that shows where the chest is from far away. No collision, no shadow. (SM_BossChestBeam)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Beam;

private:
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// A second overlap in the same frame can't give a second set of menus
	bool bCollected = false;
};
