// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "JungleCat.generated.h"

/**
 * The Jaguar Alpha's summon. On its own it's a normal chaser. In a pack (JoinPack), it runs to its place on a
 * circle around the player, circles with the rest of the pack, crouches, and at the shared LungeTime every cat
 * lunges at the player at once. After the lunge, or when the pack is released, it goes back to a normal chase.
 * A cat that's frozen or stunned when the lunge comes skips it.
 * Health, speed, contact damage and XP come from its EnemyData.
 * AttackPose for the material (same mapping as the Jaguar): 0 -> 0.5 crouch, 1 lunging.
 */
UCLASS()
class SURVIVALUNLEASHED_API AJungleCat : public AEnemyBase
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual void OnPoolAcquire_Implementation() override;

	// StartAngle: this cat's place on the circle right now (radians, around the player).
	// LungeTime: the world time the whole pack lunges.
	void JoinPack(float StartAngle, float LungeTime);

	// Back to a normal chase. A lunge already under way finishes first.
	void LeavePack();

protected:
	virtual FVector GetDesiredDirection() const override;

	UPROPERTY(EditDefaultsOnly, Category = "Jungle Cat|Pack", meta = (ClampMin = "0"))
	float CircleRadius = 500.f;

	// How fast the places move around the circle (uu/s along it)
	UPROPERTY(EditDefaultsOnly, Category = "Jungle Cat|Pack", meta = (ClampMin = "0"))
	float CircleSpeed = 250.f;

	// Within this distance of its place, a cat slows down to stay on it
	UPROPERTY(EditDefaultsOnly, Category = "Jungle Cat|Pack", meta = (ClampMin = "1"))
	float SlotSlowRadius = 150.f;

	// The pack stops and crouches for this long before the lunge, so the player sees it coming
	UPROPERTY(EditDefaultsOnly, Category = "Jungle Cat|Pack", meta = (ClampMin = "0"))
	float LungeCrouchTime = 0.3f;

	// Proposed values (not in the GDD); tune in the playtest
	UPROPERTY(EditDefaultsOnly, Category = "Jungle Cat|Pack", meta = (ClampMin = "0"))
	float LungeSpeed = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Jungle Cat|Pack", meta = (ClampMin = "0"))
	float LungeDistance = 700.f;

private:
	enum class EPackState : uint8
	{
		None,     // normal chase
		Circling,
		Lunging
	};

	EPackState PackState = EPackState::None;
	float SlotAngleAtZero = 0.f; // place on the circle at world time 0, so every cat in a pack turns together
	float PackLungeTime = 0.f;
	FVector LungeDirection = FVector::ZeroVector;
	float LungeTravelled = 0.f;

	bool IsCrouching() const;
	FVector GetSlotPoint(const FVector& PlayerLocation) const;
	void StartLunge();
	void UpdateLunge(float DeltaTime);
	void EndLunge();
};
