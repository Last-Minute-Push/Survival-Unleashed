// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/BaseWeapon.h"
#include "Enemies/HordeSubsystem.h"
#include "BaseBowToGun.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API ABaseBowToGun : public ABaseWeapon
{
	GENERATED_BODY()

	void StartAim(); // Begins aiming
	void StopAim();  // Stops aiming and fires the projectile
	inline void HideProjectile(bool bHide) { DisplayProjectile->AddLocalOffset(FVector(0.f, 0.f, bHide ? ProjectileDisappearingHeight : -ProjectileDisappearingHeight)); }

public:
	ABaseBowToGun();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void StartWeapon() override;
	virtual void StopWeapon() override;
	UFUNCTION(BlueprintCallable, Category = "Aiming") static bool FaceNearestEnemy(USceneComponent* ToRotate, UHordeSubsystem* Horde, AActor* WeaponParent, 
		                                                                           float ProjectileReach, bool bFlipRotation = false, bool bForce = true);

protected:
	float Radius;    // Data->DistanceFromPlayer
	float Range;     // Data->Range	
	float Height;    // Data->Height
	float Damage;    // Data->Damage
	float Cooldown;  // Data->Cooldown * 0.5f (one half for TFireCooldown, the other half for TAimDelay)
	float Knockback; // Data->Knockback
	float Speed;     // Data->AttackSpeed
	float Pierce;    // Data->Pierce
	FTimerHandle TFireCooldown, TAimDelay;

	UPROPERTY(Transient) TObjectPtr<UHordeSubsystem> Horde;
	UPROPERTY(VisibleAnywhere, Category = "Properties|ArrowMesh")  TObjectPtr<USkeletalMeshComponent> DisplayProjectile; // Mesh for the display arrow that appears before spawning the real moving projectile arrow
	UPROPERTY(EditDefaultsOnly, Category = "Properties|ArrowMesh") float ProjectileDisappearingHeight;                   // Height the arrow mesh will travel to "disappear"
	
};
