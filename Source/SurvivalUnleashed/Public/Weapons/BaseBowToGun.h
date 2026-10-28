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

	float Range;     // Data->Range	
	float Height;    // Data->Height
	float Radius;    // Data->Radius
	float Damage;    // Data->Damage
	float Cooldown;  // Data->Cooldown * 0.5f (one half for TFireCooldown, the other half for TAimDelay)
	float Knockback; // Data->Knockback
	float Speed;     // Data->AttackSpeed

	FTimerHandle TFireCooldown, TAimDelay;

	UPROPERTY(Transient) UHordeSubsystem* Horde;

	void StartAim(); // Begins aiming
	void StopAim();  // Stops aiming and fires the projectile
	bool FaceNearestEnemy(USceneComponent* ToRotate, bool bForce = true);

	UFUNCTION() inline void ApplyDamage(AProjectileBase* Projectile, AActor* Target) { Super::ApplyDamage((AActor*)Projectile, Target); }
	inline void HideProjectile(bool bHide) { DisplayProjectile->AddLocalOffset(FVector(0.f, 0.f, bHide ? ProjectileDisappearingHeight : -ProjectileDisappearingHeight)); }

public:
	ABaseBowToGun();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void StopWeapon() override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Properties|ArrowMesh")  USkeletalMeshComponent* DisplayProjectile; // Mesh for the display arrow that appears before spawning the real moving projectile arrow
	UPROPERTY(EditDefaultsOnly, Category = "Properties|ArrowMesh") float ProjectileDisappearingHeight;        // Height the arrow mesh will travel to "disappear"
};
