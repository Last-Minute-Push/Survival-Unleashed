// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/BaseBowToGun.h"
#include "BaseSpear.generated.h"

class UBoxComponent;

UCLASS()
class SURVIVALUNLEASHED_API ABaseSpear : public ABaseWeapon
{
	GENERATED_BODY()
	
	float MoveSpear(float DeltaTime);

	void Thrust();         // Begins the thrust cycle and activates damage overlap events.
	void Retreat();        // Finalizes the thrust cycle and deactivates damage overlap events.
	void EndCycle();       // Resets the thrust cycle and delays before the next thrust cycle.

	UFUNCTION(Category = "Damage") void ApplyDamageCollision(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	bool bIsAiming, bIsThrusting;
	float Range;     // Data->Range	
	float Height;    // Data->Height
	float Radius;    // Data->DistanceFromPlayer
	float Cooldown;  // Data->Cooldown
	float Speed;     // Data->AttackSpeed
	FTimerHandle TThrustCooldown;

	UPROPERTY(Transient) UHordeSubsystem* Horde;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Thrusting") float RetreatMultiplier = 0.25f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties|Collision") UBoxComponent* BoxCollision;
	UPROPERTY(VisibleAnywhere, Transient, Category = "Properties|Collision")          TArray<TObjectPtr<AActor>> HitActors;

	virtual void BeginPlay();
	virtual void Tick(float DeltaTime);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
	ABaseSpear();

	virtual void StopWeapon() override;
};
