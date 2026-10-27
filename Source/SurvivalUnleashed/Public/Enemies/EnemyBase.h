// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Framework/Poolable.h"
#include "EnemyBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyHit, AEnemyBase*, Enemy, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDied, AEnemyBase*, Enemy);

// Custom Primitive Data slots used by enemy materials
namespace EnemyMaterialData
{
	constexpr int32 AnimPhase = 0;
	constexpr int32 MoveAmount = 1;
	constexpr int32 HitFlash = 2;
	constexpr int32 StatusTint = 3; // slots 3-6: tint RGB + strength (A = 0 means no tint)
}

UENUM(BlueprintType)
enum class EEnemyStatus : uint8
{
	Burn,
	Freeze,
	Stun,
	Pull,
	Count UMETA(Hidden)
};

class UEnemyData;
class UHordeSubsystem;
class AEXP_PickUp;
class UCapsuleComponent;
class USkeletalMeshComponent;

UCLASS(Abstract)
class SURVIVALUNLEASHED_API AEnemyBase : public AActor, public IPoolable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemyBase();

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	virtual void OnPoolAcquire_Implementation() override;
	virtual void OnPoolRelease_Implementation() override;

	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual FVector GetVelocity() const override { return CurrentVelocity; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	UEnemyData* GetEnemyData() const { return EnemyData; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsDead() const { return bIsDead; }

	// Magnitude: damage per 0.5 s for Burn/Freeze, pull speed for Pull, unused for Stun.
	// Reapplying refreshes the duration. It never stacks.
	UFUNCTION(BlueprintCallable, Category = "Enemy|Status")
	void ApplyStatus(EEnemyStatus Status, float Duration, float Magnitude, AActor* Source = nullptr,
		FVector PullCenter = FVector::ZeroVector);

	UFUNCTION(BlueprintPure, Category = "Enemy|Status")
	bool IsStatusActive(EEnemyStatus Status) const;

	UFUNCTION(BlueprintCallable, Category = "Enemy|Material")
	void SetMaterialData(int32 Index, float Value);

	// Scales this enemy for the current run time. Call right after acquiring it from the pool.
	void ApplyDifficulty(float InHealthMultiplier, float InDamageMultiplier);

	// Pushes this enemy away from SourceLocation by Distance, reduced by KnockbackResistance
	UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
	void ApplyKnockback(const FVector& SourceLocation, float Distance);

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FOnEnemyHit OnHit;

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FOnEnemyDied OnDied;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	TObjectPtr<UEnemyData> EnemyData;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Movement", meta = (ClampMin = "0"))
	float StopDistance = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Movement", meta = (ClampMin = "0"))
	float TurnSpeed = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Animation", meta = (ClampMin = "0"))
	float MoveAmountInterpSpeed = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|HitFlash")
	float HitFlashDuration = 0.1f;

	// Returns the desired movement direction for the enemy
	virtual FVector GetDesiredDirection() const;

	FVector ComputeSeparation() const;

	// For subclasses that need their own neighbor queries (flocking)
	UHordeSubsystem* GetHorde() const { return Horde; }

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Steering", meta = (ClampMin = "0"))
	float SeekWeight = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Steering", meta = (ClampMin = "0"))
	float SeparationWeight = 1.5f;

	// Roughly 2x the capsule radius works well
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Steering", meta = (ClampMin = "1"))
	float SeparationRadius = 100.f;

	// Push priority: lighter enemies get out of the way of heavier ones
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Steering", meta = (ClampMin = "0.1"))
	float Mass = 1.f;

	// Status tints: RGB is the color, A is how much it replaces the base color
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Status")
	FLinearColor BurnTint = FLinearColor(1.f, 0.35f, 0.f, 0.6f);

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Status")
	FLinearColor FreezeTint = FLinearColor(0.55f, 0.85f, 1.f, 0.7f);

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Status")
	FLinearColor StunTint = FLinearColor(1.f, 0.9f, 0.2f, 0.5f);

	// Dropped on death, carrying EnemyData->ExperienceValue
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Loot")
	TSubclassOf<AEXP_PickUp> ExperiencePickupClass;

private:
	struct FStatusState
	{
		float TimeRemaining = 0.f;
		float Magnitude = 0.f;
		float TickAccumulator = 0.f;
		FVector PullCenter = FVector::ZeroVector;
		TWeakObjectPtr<AActor> Causer;
		TWeakObjectPtr<AController> Instigator;
	};

	static constexpr float StatusTickInterval = 0.5f;
	FStatusState Statuses[static_cast<int32>(EEnemyStatus::Count)];
	int32 LastTintStatus = INDEX_NONE; // whose tint is on the material right now

	void SetMaterialColor(int32 Index, const FLinearColor& Value);

	void UpdateStatuses(float DeltaTime);
	void ClearStatuses();
	void RefreshStatusTint();
	bool IsDisabled() const { return IsStatusActive(EEnemyStatus::Freeze) || IsStatusActive(EEnemyStatus::Stun); }


	// Largest SeparationRadius of any enemy that has begun play; widens neighbor searches
	// so small enemies can find big ones. Shared by all enemies and only ever grows.
	static float LargestSeparationRadius;

	float CurrentHealth = 0.f;
	float DamageMultiplier = 1.f;
	bool bIsDead = false;
	FVector CurrentVelocity = FVector::ZeroVector;
	FTimerHandle ContactDamageHandle;
	FTimerHandle HitFlashDurationHandle;

	FVector KnockbackVelocity = FVector::ZeroVector;
	float KnockbackTimeRemaining = 0.f;

	UPROPERTY(Transient)
	TArray<UMeshComponent*> MeshComponents;

	UPROPERTY(Transient)
	UHordeSubsystem* Horde;

	// Reused every frame to avoid allocating a new array per enemy per frame
	mutable TArray<AEnemyBase*> NeighborScratch;

	float MoveAmount = 0.f;
	float LastSentMoveAmount = 0.f;

	// Flashes the enemy for a short period of time
	void EnemyFlash();
	void RevertFlash();

	void UpdateMoveAmount(float DeltaTime);

	void DrawDebugSeparation() const;

	// Moves the enemy along its current knockback
	void UpdateKnockback(float DeltaTime);

	// Resets the enemy's state to its initial values
	void ResetState();

	// Handles the enemy's death logic
	void Die();

	// Deals contact damage to the player
	void DealContactDamage();

	// Spawns this enemy's drops at its location
	void DropLoot();

	// Handles the capsule's begin overlap event
	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Handles the capsule's end overlap event
	UFUNCTION()
	void OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Swept move that slides the leftover distance along whatever it hits (pillars, walls), staying flat
	void MoveWithSlide(const FVector & Delta);
};
