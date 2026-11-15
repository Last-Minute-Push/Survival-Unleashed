// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Framework/Poolable.h"
#include "PoisonCloud.generated.h"

class UStaticMeshComponent;

/**
 * The Spore Puffball's poison cloud. Pooled. Damages the player only: DamagePerTick every TickInterval while
 * the player stands within Radius, for Lifetime seconds. The first tick comes one interval after the burst,
 * so a player who reacts right away takes nothing.
 * The actor's origin is on the ground at the cloud's center.
 * The mesh gets Custom Primitive Data 0 = opacity (0 -> 1 as it puffs out, 1 -> 0 over the last FadeTime).
 */
UCLASS()
class SURVIVALUNLEASHED_API APoisonCloud : public AActor, public IPoolable
{
	GENERATED_BODY()

public:
	APoisonCloud();

	virtual void Tick(float DeltaTime) override;
	virtual void OnPoolAcquire_Implementation() override;
	virtual void OnPoolRelease_Implementation() override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, Category = "Poison Cloud", meta = (ClampMin = "0"))
	float Radius = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Poison Cloud", meta = (ClampMin = "0"))
	float DamagePerTick = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Poison Cloud", meta = (ClampMin = "0.05"))
	float TickInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Poison Cloud", meta = (ClampMin = "0.1"))
	float Lifetime = 5.f;

	// Seconds to puff out to full size at the start, and to fade away at the end
	UPROPERTY(EditDefaultsOnly, Category = "Poison Cloud", meta = (ClampMin = "0.01"))
	float PuffTime = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Poison Cloud", meta = (ClampMin = "0.01"))
	float FadeTime = 1.f;

private:
	FTimerHandle DamageTimer;
	FTimerHandle LifetimeTimer;
	float Age = 0.f;
	FVector MeshRestScale = FVector::OneVector; // the mesh's scale as set in the Blueprint

	void DamageTick();
	void Expire();
	void UpdateLook();
};
