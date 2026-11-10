// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/BossBase.h"
#include "JaguarAlpha.generated.h"

class UDecalComponent;
class UMaterialInstanceDynamic;
class AJungleCat;

/**
 * Second boss (10:00). Tests dodging:
 *  - Stalk: circles the player at StalkRadius, switching direction every few seconds
 *  - Pounce: crouches with a ring on the landing spot, then leaps there and hits the player if they're still inside
 *  - Call the Pack: Jungle Cats that surround the player, circle, then lunge together
 * Freeze and Stun pause the stalk and the attack timers. A leap that has started always finishes.
 * Health, speed, contact damage, XP and knockback resistance come from its EnemyData, like any enemy.
 *
 * The actor never leaves the ground: the leap's arc is drawn by the material, so drops and the chest land
 * where they should even if it dies mid-leap. AttackPose for the material:
 *    0 -> 0.5  crouch (coils down, tail lashes)
 *  0.5 -> 1    leap (stretches out; the arc's height comes from (AttackPose - 0.5) * 2)
 *   -1 -> 0    landing: absorbs the impact and eases back to standing
 */
UCLASS()
class SURVIVALUNLEASHED_API AJaguarAlpha : public ABossBase
{
	GENERATED_BODY()

public:
	AJaguarAlpha();

	virtual void Tick(float DeltaTime) override;

	// No contact damage in mid-leap: the landing is the hit
	virtual float GetContactDamage() const override;

protected:
	virtual void BeginPlay() override;
	virtual FVector GetDesiredDirection() const override;
	virtual void OnFightStarted() override;
	virtual void OnFightEnded() override;

	// ---- Stalk

	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Stalk", meta = (ClampMin = "0"))
	float StalkRadius = 600.f;

	// How far off the circle it can drift before it heads straight back to it
	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Stalk", meta = (ClampMin = "1"))
	float StalkCorrection = 150.f;

	// Seconds between changes of circling direction (random in this range)
	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Stalk", meta = (ClampMin = "0.5"))
	float StalkFlipMin = 4.f;

	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Stalk", meta = (ClampMin = "0.5"))
	float StalkFlipMax = 7.f;

	// ---- Pounce

	// Same kind of decal material as the Treant's slam: a ring with a scalar "Progress" (0 to 1)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UDecalComponent> PounceDecal;

	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pounce", meta = (ClampMin = "0.1"))
	float PounceInterval = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pounce", meta = (ClampMin = "0"))
	float PounceMinRange = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pounce", meta = (ClampMin = "0"))
	float PounceMaxRange = 900.f;

	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pounce", meta = (ClampMin = "0.01"))
	float PounceCrouchTime = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pounce", meta = (ClampMin = "0.01"))
	float PounceLeapTime = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pounce", meta = (ClampMin = "0"))
	float PounceRadius = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pounce", meta = (ClampMin = "0"))
	float PounceDamage = 25.f;

	// Standing still after landing, while the material eases out of the landing pose
	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pounce", meta = (ClampMin = "0.01"))
	float PounceRecoverTime = 0.3f;

	// ---- Call the Pack

	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pack")
	TSubclassOf<AJungleCat> CatClass;

	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pack", meta = (ClampMin = "0.1"))
	float PackInterval = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pack", meta = (ClampMin = "1"))
	int32 PackSize = 4;

	// Cats appear this far from the Jaguar, then run to their places around the player
	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pack", meta = (ClampMin = "0"))
	float PackSpawnRadius = 300.f;

	// Time for the cats to reach the circle, then time spent circling before the shared lunge
	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pack", meta = (ClampMin = "0"))
	float PackGatherTime = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pack", meta = (ClampMin = "0"))
	float PackCircleTime = 3.f;

	// Cats made up front (only once, shared by every Jaguar)
	UPROPERTY(EditDefaultsOnly, Category = "Jaguar|Pack", meta = (ClampMin = "0"))
	int32 CatPrewarm = 8;

private:
	enum class EJaguarState : uint8
	{
		Stalk,
		Crouch,  // the ring is down, the landing spot is locked
		Leap,    // committed: always finishes
		Recover  // just landed
	};

	EJaguarState State = EJaguarState::Stalk;
	float StateElapsed = 0.f;

	float PounceCooldown = 0.f;
	float PackCooldown = 0.f;
	float FlipCooldown = 0.f;
	float OrbitSign = 1.f; // +1 counterclockwise, -1 clockwise

	FVector LeapStart = FVector::ZeroVector;
	FVector LeapEnd = FVector::ZeroVector; // capsule center at the landing spot

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PounceDecalMaterial;

	// The cats from the latest call, so they can be released when the fight ends
	TArray<TWeakObjectPtr<AJungleCat>> Pack;

	bool CanAdvanceTimers() const;
	bool TryStartPounce();
	void StartLeap();
	void UpdateLeap(float DeltaTime);
	void Land();
	void CallThePack();
	void ReleasePack();

	// The landing point nearest Target, along the line back to Start, with room for the capsule
	bool FindLandingSpot(const FVector& Start, const FVector& Target, FVector& OutCenter) const;

	void SetPounceTelegraph(bool bVisible, float Progress = 0.f);
	void TurnTowards(const FVector& Point, float DeltaTime);
	void ResetFlipTimer();
};
