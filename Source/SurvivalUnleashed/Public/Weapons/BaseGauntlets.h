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

	UPROPERTY(BlueprintReadOnly) float                                                Direction;
	UPROPERTY(BlueprintReadOnly) FGauntletState&                                      State;
	UPROPERTY(BlueprintReadOnly) TWeakObjectPtr<FGauntlet>                            Other;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<USceneComponent>        GauntletPivot;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<USkeletalMeshComponent> Mesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UBoxComponent>          BoxCollision;

	FGauntlet(FGauntletState& _State) : State(_State) {}
};

struct FGauntletState
{
	void (ABaseGauntlets::*Start)(FGauntlet&);
	void (ABaseGauntlets::*Update)(FGauntlet&, float);
	void (ABaseGauntlets::*Exit)(FGauntlet&);

	FGauntletState(void (ABaseGauntlets::*StartFP)(FGauntlet&), void (ABaseGauntlets::*UpdateFP)(FGauntlet&, float), void (ABaseGauntlets::*ExitFP)(FGauntlet&))
	{
		Start = StartFP;
		Update = UpdateFP;
		Exit = ExitFP;
	}

	FGauntletState() : Start(nullptr), Update(nullptr), Exit(nullptr) {}
};

/**
 * Matrix4x4 (*RotationAxis)(float) = YRotationMatrix;
 */
UCLASS()
class SURVIVALUNLEASHED_API ABaseGauntlets : public ABaseWeapon
{
	GENERATED_BODY() 

	int32 PunchCount = 0;

	// Start Functions
	void Punch(FGauntlet& Gauntlet);                                                // STATE_PUNCHING
	void Retreat(FGauntlet& Gauntlet);                                              // STATE_RETREAT

	// Update Functions
	void MoveGauntlets(FGauntlet& Gauntlet, float DeltaTime);                       // STATE_PUNCHING, STATE_RETREAT

	// Exit Functions
	void EndPunch(FGauntlet& Gauntlet);                                             // STATE_PUNCHING
	void EndRetreat(FGauntlet& Gauntlet);		                                    // STATE_RETREAT

protected:
	float Range;     // Data->Range	
	float Height;    // Data->Height
	float Radius;    // Data->Radius
	float Cooldown;  // Data->Cooldown
	FTimerHandle TPunchCooldown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties") FGauntlet Left;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties") FGauntlet Right;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties") TWeakObjectPtr<FGauntlet> First;

	UPROPERTY(Transient) UHordeSubsystem* Horde;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|States")                            FGauntletState StatePunch;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|States")                            FGauntletState StateHitDelay;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|States")                            FGauntletState StateRetreat;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|States")                            FGauntletState StateCooldown;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Punching", meta = (ClampMin = "1")) int32 PunchCountMax = 2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Punching", meta = (ClampMin = "0")) float ArcRadius = 60.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Punching", meta = (ClampMin = "0")) float PunchHitDelay = 0.25f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Punching")                          USceneComponent* GauntletPivotLeft;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Punching")                          USceneComponent* GauntletPivotRight;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Mesh")                              float DistanceBetweenGauntlets = 20.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Mesh")                              USkeletalMeshComponent* MeshLeft;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Mesh")                              USkeletalMeshComponent* MeshRight;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Collision")                         UBoxComponent* BoxCollisionLeft;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Collision")                         UBoxComponent* BoxCollisionRight;
	UPROPERTY(VisibleAnywhere, Transient, Category = "Properties|Collision")                                  TArray<TObjectPtr<AActor>> HitActors;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:
	ABaseGauntlets();

	virtual void StopWeapon() override;
};
