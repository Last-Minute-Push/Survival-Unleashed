// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/BaseWeapon.h"
#include "BaseGauntlets.generated.h"

class UBoxComponent;
class UGauntletData;
class UHordeSubsystem;

DECLARE_DELEGATE_OneParam(FOnStartState, UGauntletData* /*Gauntlet*/);
DECLARE_DELEGATE_TwoParams(FOnUpdateState, UGauntletData* /*Gauntlet*/, float /*DeltaTime*/);
DECLARE_DELEGATE_OneParam(FOnExitState, UGauntletData* /*Gauntlet*/);

// Change the parameters from & to *, createdefaultsubobject the UGauntletData (hope it works), and make sure it's pointing to the same memory address by checking if lol changed

USTRUCT(BlueprintType)
struct FGauntletState
{
	GENERATED_BODY()

	FOnStartState Start;
	FOnUpdateState Update;
	FOnExitState Exit;

	FGauntletState() {};

	FGauntletState(UObject* UserObject, FName StartName, FName UpdateName, FName ExitName)
	{
		Start.BindUFunction(UserObject, StartName);
		Update.BindUFunction(UserObject, UpdateName);
		Exit.BindUFunction(UserObject, ExitName);
	}

	~FGauntletState()
	{
		Start.Unbind();
		Update.Unbind();
		Exit.Unbind();
	}
};

/**
 * Matrix4x4 (*RotationAxis)(float) = YRotationMatrix;
 */
UCLASS()
class SURVIVALUNLEASHED_API ABaseGauntlets : public ABaseWeapon
{
	GENERATED_BODY() 

	int32 PunchCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Properties") UGauntletData* Left;
	UPROPERTY(EditDefaultsOnly, Category = "Properties") UGauntletData* Right;
	UPROPERTY(EditDefaultsOnly, Category = "Properties") UGauntletData* First;

	UGauntletData* GetOther(UGauntletData* Gauntlet) { return (Gauntlet == Left) ? Right : Left; }
	void SetState(UGauntletData* Gauntlet, FGauntletState* State);

	// Start Functions
	UFUNCTION(BlueprintCallable) void Punch(UGauntletData* Gauntlet);                          // STATE_PUNCHING
	UFUNCTION(BlueprintCallable) void Retreat(UGauntletData* Gauntlet);                        // STATE_RETREAT

	// Update Functions
	UFUNCTION(BlueprintCallable) void MoveGauntlets(UGauntletData* Gauntlet, float DeltaTime); // STATE_PUNCHING, STATE_RETREAT

	// Exit Functions
	UFUNCTION(BlueprintCallable) void EndPunch(UGauntletData* Gauntlet);                       // STATE_PUNCHING
	UFUNCTION(BlueprintCallable) void EndRetreat(UGauntletData* Gauntlet);		               // STATE_RETREAT

	UFUNCTION(BlueprintCallable) void ApplyDamageCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		                                                   int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	float Range;     // Data->Range	
	float Height;    // Data->Height
	float Radius;    // Data->Radius
	float Cooldown;  // Data->Cooldown
	FTimerHandle TPunchCooldown;


	UPROPERTY(Transient) UHordeSubsystem* Horde;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|States")                            FGauntletState StatePunch;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|States")                            FGauntletState StateHitDelay;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|States")                            FGauntletState StateRetreat;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|States")                            FGauntletState StateCooldown;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Punching", meta = (ClampMin = "1")) int32 PunchCountMax = 2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Punching", meta = (ClampMin = "0")) float ArcRadius = 60.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Punching", meta = (ClampMin = "0")) float PunchHitDelay = 0.25f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Punching", meta = (ClampMin = "0")) float EnemyFacingReach = 2400.f;
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