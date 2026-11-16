// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDatabase.generated.h"

namespace WeaponName
{
	constexpr const char Sword[] = "Sword";
	constexpr const char BowToGun[] = "Bow-To-Gun";
	constexpr const char Axe[] = "Axe";
	constexpr const char Spear[] = "Spear";
	constexpr const char FireStaff[] = "Fire Staff";
	constexpr const char IceStaff[] = "Ice Staff";
	constexpr const char LightningStaff[] = "Lightning Staff";
	constexpr const char EarthStaff[] = "Earth Staff";
	constexpr const char Gauntlets[] = "Gauntlets";
}

UENUM(BlueprintType)
enum class EWeaponCategory : uint8
{
	EBP_Sword          UMETA(DisplayName = "Sword"),
	EBP_BowToGun       UMETA(DisplayName = "Bow-To-Gun"),
	EBP_Axe            UMETA(DisplayName = "Axe"),
	EBP_Spear          UMETA(DisplayName = "Spear"),
	EBP_FireStaff      UMETA(DisplayName = "Fire Staff"),
	EBP_IceStaff       UMETA(DisplayName = "Ice Staff"),
	EBP_LightningStaff UMETA(DisplayName = "Lightning Staff"),
	EBP_EarthStaff     UMETA(DisplayName = "Earth Staff"),
	EBP_OfferEnd       UMETA(Hidden), // Any category below will not be offered by the level up screen.
	EBP_Gauntlets      UMETA(DisplayName = "Gauntlets")
};
ENUM_RANGE_BY_COUNT(EWeaponCategory, EWeaponCategory::EBP_OfferEnd);

class ABaseWeapon;

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UWeaponDatabase : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// Any weapon upgrades equal or above the index respective
	// to their category will be considered an evolution of that weapon.
	UPROPERTY(EditDefaultsOnly) int32 FirstEvolutionIndexSword          = -1;
	UPROPERTY(EditDefaultsOnly) int32 FirstEvolutionIndexBowToGun       = -1;
	UPROPERTY(EditDefaultsOnly) int32 FirstEvolutionIndexAxe            = -1;
	UPROPERTY(EditDefaultsOnly) int32 FirstEvolutionIndexSpear          = -1;
	UPROPERTY(EditDefaultsOnly) int32 FirstEvolutionIndexGauntlets      = -1;
	UPROPERTY(EditDefaultsOnly) int32 FirstEvolutionIndexFireStaff      = -1;
	UPROPERTY(EditDefaultsOnly) int32 FirstEvolutionIndexIceStaff       = -1;
	UPROPERTY(EditDefaultsOnly) int32 FirstEvolutionIndexLightningStaff = -1;
	UPROPERTY(EditDefaultsOnly) int32 FirstEvolutionIndexEarthStaff     = -1;

	UPROPERTY(EditDefaultsOnly) TArray<TSubclassOf<ABaseWeapon>> SwordUpgrades;
	UPROPERTY(EditDefaultsOnly) TArray<TSubclassOf<ABaseWeapon>> BowToGunUpgrades;
	UPROPERTY(EditDefaultsOnly) TArray<TSubclassOf<ABaseWeapon>> AxeUpgrades;
	UPROPERTY(EditDefaultsOnly) TArray<TSubclassOf<ABaseWeapon>> SpearUpgrades;
	UPROPERTY(EditDefaultsOnly) TArray<TSubclassOf<ABaseWeapon>> GauntletsUpgrades;
	// Staves: list the same Blueprint once per level (6 times). The upgrade index picks the level's row in its StaffData.
	UPROPERTY(EditDefaultsOnly) TArray<TSubclassOf<ABaseWeapon>> FireStaffUpgrades;
	UPROPERTY(EditDefaultsOnly) TArray<TSubclassOf<ABaseWeapon>> IceStaffUpgrades;
	UPROPERTY(EditDefaultsOnly) TArray<TSubclassOf<ABaseWeapon>> LightningStaffUpgrades;
	UPROPERTY(EditDefaultsOnly) TArray<TSubclassOf<ABaseWeapon>> EarthStaffUpgrades;
};
