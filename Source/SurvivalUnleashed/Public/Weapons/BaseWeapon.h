// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "WeaponDatabase.h"
#include "GameFramework/Actor.h"

#include "BaseWeapon.generated.h"

class UWeaponData;
class USceneComponent;
class AProjectileBase;
class UPlayerStatsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponHit, float, Damage);

UCLASS(Abstract)
class SURVIVALUNLEASHED_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	

public:

	// Sets default values for this actor's properties
	ABaseWeapon();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(BlueprintAssignable, Category = "Damage") FOnWeaponHit OnWeaponHit;
	
	UFUNCTION(BlueprintCallable, Category = "Weapon") virtual void StopWeapon();
	UFUNCTION(BlueprintCallable, Category = "Weapon") static float ComputeWeaponHit(UWeaponData* W, UPlayerStatsComponent* P/*, UEnemyStats* E*/);
	static float ComputeHitFromBase(float BaseDamage, UPlayerStatsComponent* P); // same formula, for weapons whose damage changes per level
	UFUNCTION(BlueprintCallable, Category = "Weapon") inline bool AbsoluteRotation() const { return bAbsoluteRotation; }

	UFUNCTION(BlueprintCallable, Category = "WeaponType") inline EWeaponCategory GetEWeaponCategory() const { return Type; }
	inline void SetWeaponCategory(EWeaponCategory InType) { Type = InType; }
	UFUNCTION(BlueprintCallable, Category = "WeaponType") inline int32 GetWeaponUpgrade() const { return Upgrade; }
	UFUNCTION(BlueprintCallable, Category = "WeaponType") inline void SetWeaponUpgrade(int32 U) { Upgrade = U; }

	UFUNCTION(BlueprintCallable, Category = "Stats") void SetParent(AUC_Player* P);
	UFUNCTION(BlueprintCallable, Category = "Stats") inline UWeaponData* GetWeaponData() const { return Data; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Properties") bool bAbsoluteRotation;                  // Rotates on its own regard
	UPROPERTY(VisibleAnywhere, Category = "Properties|WeaponType") EWeaponCategory Type;          // Identifies the category of this weapon
	UPROPERTY(VisibleAnywhere, Category = "Properties|WeaponType") int32 Upgrade;                 // Identifies the upgrade of this weapon
	UPROPERTY(VisibleAnywhere, Category = "Properties|Mesh") USkeletalMeshComponent* Mesh;        // Weapon's mesh
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Stats") UWeaponData* Data;                 // Weapon's own data
	UPROPERTY(VisibleAnywhere, Category = "Properties|Stats") AUC_Player* Parent;                 // Reference to the player parent 
	UPROPERTY(VisibleAnywhere, Category = "Properties|Stats") UPlayerStatsComponent* ParentStats; // Reference to the player parent's stats component
	UPROPERTY(VisibleAnywhere, Category = "Properties|Components") USceneComponent* Pivot;        // Pivot used for the swinging motion; rotates only the sword and collision box

	virtual void ApplyDamage(AActor* Source, AActor* Target);

	// Damage for one hit before the target. Override when damage doesn't come straight from Data->Damage.
	virtual float GetHitDamage() const;
};