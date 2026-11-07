// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "PitcherPlant.generated.h"

class AProjectileBase;
class UStaticMeshComponent;

/**
 * Ranged specialist (08:00). Tunnels underground to a spot 800-1,200 uu from the player, surfaces, roots,
 * and spits acid at the player's current position. If the player gets within 500 uu it burrows and moves away.
 *
 * Burrowed: collision is off, so weapons, projectiles and contact damage can't touch it, and it tunnels under
 * obstacles. Only a dirt mound shows. TakeDamage is also ignored, for anything that reaches it some other way.
 * Health, contact damage, XP and knockback resistance come from its EnemyData. Its Move Speed is the burrowed speed.
 */
UCLASS()
class SURVIVALUNLEASHED_API APitcherPlant : public AEnemyBase
{
	GENERATED_BODY()

public:
	APitcherPlant();

	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	virtual void OnPoolAcquire_Implementation() override;

	virtual bool IsTargetable() const override;

protected:
	virtual void BeginPlay() override;
	virtual FVector GetDesiredDirection() const override;

	// Shown while underground or coming up. Its origin should sit on the ground.
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MoundMesh;

	// ---- Burrowing

	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Burrow", meta = (ClampMin = "0"))
	float PreferredRangeMin = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Burrow", meta = (ClampMin = "0"))
	float PreferredRangeMax = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Burrow", meta = (ClampMin = "0.01"))
	float SurfaceTime = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Burrow", meta = (ClampMin = "0.01"))
	float BurrowTime = 0.5f;

	// How far the plant's meshes drop below the floor while underground
	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Burrow", meta = (ClampMin = "0"))
	float SinkDepth = 250.f;

	// Player this close (and the cooldown ready): burrow and move away
	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Burrow", meta = (ClampMin = "0"))
	float RepositionRange = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Burrow", meta = (ClampMin = "0"))
	float RepositionCooldown = 6.f;

	// Player out of acid range for this long (and the cooldown ready)
	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Burrow", meta = (ClampMin = "0"))
	float OutOfRangeTime = 3.f;

	// ---- Acid

	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Acid")
	TSubclassOf<AProjectileBase> AcidClass;

	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Acid", meta = (ClampMin = "0.1"))
	float SpitInterval = 2.5f;

	// After surfacing, before the first spit
	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Acid", meta = (ClampMin = "0"))
	float FirstSpitDelay = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Acid", meta = (ClampMin = "0"))
	float AcidDamage = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Acid", meta = (ClampMin = "0"))
	float AcidSpeed = 900.f;

	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Acid", meta = (ClampMin = "0"))
	float AcidRange = 1400.f;

	// Where the acid leaves the pitcher, relative to the actor (the capsule's center)
	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Acid")
	FVector MouthOffset = FVector(0.f, 0.f, 60.f);

	// Wind-up before each spit: the lid opens and the pitcher leans back (material AttackPose 0 -> 1)
	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Acid", meta = (ClampMin = "0.01"))
	float SpitWindup = 0.35f;

	// On firing, the pose jumps to this (leaning forward) and eases back to rest over SpitRecoverTime
	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Acid")
	float SpitLungePose = -0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "Pitcher|Acid", meta = (ClampMin = "0.01"))
	float SpitRecoverTime = 0.3f;

private:
	enum class EState : uint8
	{
		Burrowed,   // underground, untargetable, tunneling to BurrowTarget
		Surfacing,  // coming up; targetable
		Rooted,     // above ground, spitting
		Burrowing   // going down; still targetable until it's under
	};

	EState State = EState::Burrowed;
	float StateTime = 0.f;

	FVector BurrowTarget = FVector::ZeroVector;
	bool bHasBurrowTarget = false;
	float RetargetTimer = 0.f;

	float SpitCooldown = 0.f;
	float RepositionCooldownRemaining = 0.f;
	float OutOfRangeTimer = 0.f;
	float SpitRecoverRemaining = 0.f;

	// The plant's own meshes and where the Blueprint put them, so they can be sunk below the floor
	struct FVisual
	{
		TWeakObjectPtr<USceneComponent> Component;
		float RestZ = 0.f;
	};
	TArray<FVisual> Visuals;

	void SetState(EState NewState);
	void ApplyCollision();
	void UpdateVisuals();

	void TickBurrowed(const FVector& PlayerLocation);
	void TickRooted(const APawn* Player, float DeltaTime);

	bool PickBurrowTarget(const FVector& PlayerLocation);
	bool IsSpotClear(const FVector& Point, FVector& OutGround) const;

	void SpitAcid(const APawn* Player);
	void HandleAcidHit(AProjectileBase* Projectile, AActor* Target);

	// Material pose for the spit: wind-up as the cooldown runs out, lunge after firing
	void UpdateSpitPose(bool bInRange, float DeltaTime);
};
