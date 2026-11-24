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

	void StartSwing();
	void StopSwing();

	UFUNCTION(Category = "Damage") void ApplyDamageCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		                                                     int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(Category = "Damage") void BloodrootAxeHeal(float Damage);

	void Swing(bool bRight, float DeltaTime);
	void HideSword(bool bHide);

protected:
	bool bIsHidden;
	bool bIsSwinging;
	float HealAmount;
	float Next, Prev, Difference;
	float Height;
	float Radius;
	float SwingAngle;
	float SwingSpeed;             // Data->AttackSpeed
	float SwingArc;               // Data->Range
	float PastRangeRight;
	float PastRangeLeft;
	float SwingPatternCooldown;   // Data->Cooldown
	uint32 CurrentSwingCount, EvolutionLevel;
	FTimerHandle TSwingStart, TDisappearDelay;

	UPROPERTY(EditDefaultsOnly, Category = "Properties|Swing", meta = (ClampMin = "0")) uint32 SwingCountMax;                      // Total number of swings in one set
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Swing", meta = (ClampMin = "0")) float SwingInterval;                       // Delay between individual swings
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Swing")                          float SwingDisappearingDistance;           // Distance the sword will travel underground to "disappear"
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Swing", meta = (ClampMin = "0")) float SwingDisappearingDelay;              // Delay before sword disappears after a complete set of swings

	UPROPERTY(EditDefaultsOnly, Category = "Properties|Collision")                      float CollisionHeightOffset;               // Height offset for the sword's collision box
	UPROPERTY(VisibleAnywhere, Transient, Category = "Properties|Collision")            TArray<TObjectPtr<AActor>> HitActors;      // Collection of enemies that were previously hit by the sword's current swing
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Components")                     class UBoxComponent* BoxCollision;         // Collision used to hit enemies between swings
	UPROPERTY(VisibleAnywhere, Category = "Properties|Niagara")                         class UNiagaraComponent* NiagaraComponent; // Niagara swing effect for the sword

public:
	ABaseSword();

	UPROPERTY(BlueprintAssignable, Category = "Properties|Swing|Event") FOnSwordEvent OnSwingEnd;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void StartWeapon() override;
	virtual void StopWeapon() override;

	UFUNCTION(Category = "Type") inline int32 GetEvolutionLevel()        const { return EvolutionLevel; }

	UFUNCTION(Category = "Components") inline float GetHeight()          const { return Height; }
	UFUNCTION(Category = "Components") inline float GetRadius()          const { return Radius; }
	UFUNCTION(Category = "Components") inline float GetCollisionHeight() const{ return CollisionHeightOffset; }

	UFUNCTION(Category = "Swing") inline bool IsHidden()                 const { return bIsHidden; }
	UFUNCTION(Category = "Swing") inline bool IsSwinging()               const { return bIsSwinging; }
	UFUNCTION(Category = "Swing") inline float GetSwingSpeed()           const { return SwingSpeed; }
	UFUNCTION(Category = "Swing") inline float GetSwingArc()             const { return SwingArc; }
	UFUNCTION(Category = "Swing") inline float GetSwingInterval()        const { return SwingInterval; }
	UFUNCTION(Category = "Swing") inline float GetDisappearingDelay()    const { return SwingDisappearingDelay; }
	UFUNCTION(Category = "Swing") inline float GetCooldown()             const { return SwingPatternCooldown; }
	UFUNCTION(Category = "Swing") inline float GetDisappearingDistance() const { return SwingDisappearingDistance; }
	UFUNCTION(Category = "Swing") inline int32 GetSwingCountMax()        const { return SwingCountMax; }
	UFUNCTION(Category = "Swing") inline int32 GetSwingCount()           const { return CurrentSwingCount; }
	UFUNCTION(Category = "Swing") inline float GetNext()                 const { return Next; }
	UFUNCTION(Category = "Swing") inline float GetPrev()                 const { return Prev; }
	UFUNCTION(Category = "Swing") inline float GetDifference()           const { return Difference; }
	UFUNCTION(Category = "Swing") inline float GetSwingAngle()           const { return SwingAngle; }
	UFUNCTION(Category = "Swing") inline float GetPastRangeLeft()        const { return PastRangeLeft; }
	UFUNCTION(Category = "Swing") inline float GetPastRangeRight()       const { return PastRangeRight; }
	UFUNCTION(Category = "Swing") inline int32 GetHitCount()             const { return HitActors.Num(); }
};