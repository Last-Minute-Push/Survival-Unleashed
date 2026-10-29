// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/BaseBowToGun.h"
#include "BaseSpear.generated.h"

class UBoxComponent;

UCLASS()
class SURVIVALUNLEASHED_API ABaseSpear : public ABaseBowToGun
{
	GENERATED_BODY()
	
	void StartThrust();    // Begins the thrust cycle and activates damage overlap events.
	void StopThrust();     // Finalizes the thrust cycle and deactivates damage overlap events.
	void EndThrustCycle(); // Resets the thrust cycle and delays before the next thrust cycle.

	UFUNCTION(Category = "Damage") void ApplyDamageCollision(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	bool bIsAiming, bIsThrusting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Thrusting") float ThrustRetreatMultiplier = 0.25f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Collision") UBoxComponent* BoxCollision;
	UPROPERTY(VisibleAnywhere, Transient, Category = "Properties|Collision")          TArray<TObjectPtr<AActor>> HitActors;

	virtual void BeginPlay();
	virtual void Tick(float DeltaTime);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
	ABaseSpear();

	virtual void StopWeapon() override;
};
