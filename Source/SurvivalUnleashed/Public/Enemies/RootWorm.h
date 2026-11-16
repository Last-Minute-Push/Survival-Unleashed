// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "RootWorm.generated.h"

class UDecalComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Ambush specialist (11:00). Tunnels toward the player underground: can't be targeted or damaged and shows only
 * a soil ripple. Within AmbushRange it locks the player's position, shows a warning ring there for WarningTime
 * and races to it underground, then erupts: damage and knockback to the player if they're still in the ring.
 * It stays up for SurfaceTime (the only time it can be hit), then burrows again and waits AmbushCooldown.
 * Freeze and Stun pause the warning and the time above ground.
 * The material reads AttackPose as how far out of the ground it is (0 buried -> 1 fully out).
 */
UCLASS()
class SURVIVALUNLEASHED_API ARootWorm : public AEnemyBase
{
	GENERATED_BODY()

public:
	ARootWorm();

	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;
	virtual void OnPoolAcquire_Implementation() override;

	// Only while above ground
	virtual bool IsTargetable() const override;
	virtual float GetContactDamage() const override;

protected:
	virtual void BeginPlay() override;
	virtual FVector GetDesiredDirection() const override;

	// Warning ring at the locked spot. Absolute location, so it stays put while the worm moves.
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UDecalComponent> RingDecal;

	// Soil ripple on the ground above the worm while it's underground
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> RippleMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Root Worm", meta = (ClampMin = "0"))
	float AmbushRange = 450.f;

	// Radius of the warning ring and of the eruption
	UPROPERTY(EditDefaultsOnly, Category = "Root Worm", meta = (ClampMin = "1"))
	float AmbushRadius = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Root Worm", meta = (ClampMin = "0.01"))
	float WarningTime = 1.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Root Worm", meta = (ClampMin = "0"))
	float EruptionDamage = 18.f;

	// Launch speed (uu/s) away from the ring's center. About 600 pushes the player roughly 300 uu.
	UPROPERTY(EditDefaultsOnly, Category = "Root Worm", meta = (ClampMin = "0"))
	float KnockbackSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Root Worm", meta = (ClampMin = "0.1"))
	float SurfaceTime = 2.f;

	// Seconds to come out at the start of SurfaceTime, and to sink back at the end
	UPROPERTY(EditDefaultsOnly, Category = "Root Worm", meta = (ClampMin = "0.01"))
	float EmergeTime = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "Root Worm", meta = (ClampMin = "0.01"))
	float BurrowTime = 0.3f;

	// Tunnel time after burrowing before it can ambush again
	UPROPERTY(EditDefaultsOnly, Category = "Root Worm", meta = (ClampMin = "0"))
	float AmbushCooldown = 5.f;

	// So it doesn't ambush the moment it spawns
	UPROPERTY(EditDefaultsOnly, Category = "Root Worm", meta = (ClampMin = "0"))
	float FirstAmbushDelay = 1.f;

	// Scalar parameter the ring decal material fills from 0 to 1 over the warning
	UPROPERTY(EditDefaultsOnly, Category = "Root Worm")
	FName RingProgressParam = TEXT("Progress");

private:
	enum class EWormState : uint8
	{
		Tunneling, // chasing underground
		Warning,   // ring is down, racing to it underground
		Surfaced   // above ground, can be hit
	};

	EWormState State = EWormState::Tunneling;
	float StateElapsed = 0.f;
	float CooldownRemaining = 0.f;

	FVector WarningStart = FVector::ZeroVector;  // where the worm was when it locked on
	FVector AmbushLocation = FVector::ZeroVector; // the locked spot, at the worm's height

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> RingMaterial;

	bool CanAdvanceTimers() const;
	void StartWarning(const APawn& Player);
	void Erupt();
	void Burrow();
	float GetHalfHeight() const;
	void SetRingVisible(bool bVisible, float Progress = 0.f);
};
