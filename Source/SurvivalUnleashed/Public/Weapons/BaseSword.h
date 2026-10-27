// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Weapons/BaseWeapon.h"

#include "BaseSword.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSwordEvent);

/**
 *
 */
UCLASS()
class SURVIVALUNLEASHED_API ABaseSword : public ABaseWeapon
{
	GENERATED_BODY()

	bool bIsSwinging;
	float Height;
	float Radius;
	float SwingAngle;
	float SwingSpeed;             // Data->AttackSpeed
	float SwingArc;               // Data->Range
	float SwingArcHalf;
	float SwingPatternCooldown;   // Data->Cooldown
	uint32 CurrentSwingCount;

	void StartSwing();
	void StopSwing();

	UFUNCTION(BlueprintCallable, Category = "Damage") void ApplyDamageCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		                                                               int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void Swing(bool bRight, float DeltaTime) const;
	inline void HideSword(bool bHide) const { Pivot->AddLocalOffset(FVector(0.f, 0.f, bHide ? SwingDisappearingDistance : -SwingDisappearingDistance)); }

public:
	ABaseSword();

	UPROPERTY(BlueprintAssignable, Category = "Properties|Swing|Event") FOnSwordEvent OnSwingStart;
	UPROPERTY(BlueprintAssignable, Category = "Properties|Swing|Event") FOnSwordEvent OnSwingEnd;
	// TODO: Create an event that takes in the final array of enemies that got hit in the arc after a swing (for points?)

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "Swing") virtual bool IsSwinging() const { return bIsSwinging; }
	virtual void StopWeapon() override;

protected:
	FTimerHandle TSwingStart, TDisappearDelay;
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Swing", meta = (ClampMin = "0")) uint32 SwingCountMax;                      // Total number of swings in one set
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Swing", meta = (ClampMin = "0")) float SwingInterval;                       // Delay between individual swings
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Swing")                          float SwingDisappearingDistance;           // Distance the sword will travel underground to "disappear"
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Swing", meta = (ClampMin = "0")) float SwingDisappearingDelay;              // Delay before sword disappears after a complete set of swings

	UPROPERTY(EditDefaultsOnly, Category = "Properties|Collision")                      float CollisionHeightOffset;               // Height offset for the sword's collision box
	UPROPERTY(VisibleAnywhere, Transient, Category = "Properties|Collision")            TArray<TObjectPtr<AActor>> HitActors;      // Collection of enemies that were previously hit by the sword's current swing
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Components")                     class UBoxComponent* BoxCollision;         // Collision used to hit enemies between swings
	UPROPERTY(VisibleAnywhere, Category = "Properties|Niagara")                         class UNiagaraComponent* NiagaraComponent; // Niagara swing effect for the sword
	
};