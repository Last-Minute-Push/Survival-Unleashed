// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/BaseBowToGun.h"
#include "BaseGauntlets.generated.h"

class UBoxComponent;

USTRUCT(BlueprintType)
struct FGauntlet
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) bool                               bIsActive = false;
	UPROPERTY(BlueprintReadOnly) bool                               bIsPunching = false;
	UPROPERTY(BlueprintReadOnly) float                              Direction;
	UPROPERTY(BlueprintReadOnly) TObjectPtr<USceneComponent>        GauntletPivot;
	UPROPERTY(BlueprintReadOnly) TObjectPtr<USkeletalMeshComponent> Mesh;
	UPROPERTY(BlueprintReadOnly) TObjectPtr<UBoxComponent>          BoxCollision;
};

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API ABaseGauntlets : public ABaseWeapon
{
	GENERATED_BODY() 

	int32 PunchCount = 0;


	// Gauntlet States
	void Punch(FGauntlet& Gauntlet);
	void Retreat(FGauntlet& Gauntlet);
	void EndCycle();

	void MoveGauntlets();
		
	inline FGauntlet& GetOther(FGauntlet& Gauntlet) { return (Gauntlet.GauntletPivot == Left.GauntletPivot) ? Right : Left; }

protected:
	bool bIsAiming;
	float Range;     // Data->Range	
	float Height;    // Data->Height
	float Radius;    // Data->Radius
	float Cooldown;  // Data->Cooldown
	FTimerHandle TPunchCooldown;

	UPROPERTY(EditDefaultsOnly, Category = "Properties") FGauntlet Left;
	UPROPERTY(EditDefaultsOnly, Category = "Properties") FGauntlet Right;

	UPROPERTY(Transient) UHordeSubsystem* Horde;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Punching")                           int32 PunchCountMax = 2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Punching", meta = (ClampMin = "0"))  float ArcRadius = 60.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Punching")                           float RetreatMultiplier = 0.25f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Punching")                           float AfterPunchDelay = 0.25f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Mesh")                               float DistanceBetweenGauntlets = 20.f;
	UPROPERTY(VisibleAnywhere, Transient, Category = "Properties|Collision")                                   TArray<TObjectPtr<AActor>> HitActors;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:
	ABaseGauntlets();

	virtual void StopWeapon() override;
};
