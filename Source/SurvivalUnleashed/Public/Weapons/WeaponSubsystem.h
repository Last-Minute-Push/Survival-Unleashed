// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UI/UpgradeCard.h"
#include "WeaponSubsystem.generated.h"

UENUM(BlueprintType)
enum class EWeaponUpgradeStatus : uint8
{
	E_Success    UMETA(DisplayName = "Success"),
	E_MaxUpgrade UMETA(DisplayName = "Max Upgrade"),
	E_MaxSlots   UMETA(DisplayName = "MaxSlots"),
	E_Error      UMETA(DisplayName = "Error")
};

class AUC_Player;

USTRUCT(BlueprintType)
struct FWeaponCreateParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) TWeakObjectPtr<USceneComponent> Attachment;
	UPROPERTY(EditAnywhere) TWeakObjectPtr<AUC_Player> Player;
	UPROPERTY(EditAnywhere) EWeaponCategory Type;
	UPROPERTY(EditAnywhere) int32 DebugUpgrade = -1; // Left at negative one if not debugging a weapon
	UPROPERTY(EditAnywhere) bool bStartWeapon;
};

/**
 *
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponsUI, EWeaponCategory, Category, int, Level);
UCLASS()
class SURVIVALUNLEASHED_API UWeaponSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	UWeaponDatabase* Database;

	EWeaponUpgradeStatus CreateWeapon(FWeaponCreateParams& Params, TMap<EWeaponCategory, UChildActorComponent*>& Inventory, TArray<TSubclassOf<ABaseWeapon>>& Upgrades);
	EWeaponUpgradeStatus UpgradeWeapon(FWeaponCreateParams& Params, TMap<EWeaponCategory, UChildActorComponent*>& Inventory, TWeakObjectPtr<UChildActorComponent> WeaponComponent, TArray<TSubclassOf<ABaseWeapon>>& Upgrades);

public:

	UPROPERTY()
	FOnWeaponsUI OnWeaponsUIUpdate;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	UFUNCTION(BlueprintCallable, Category = "Tests") void WeaponSubsystemTest(FWeaponCreateParams& Params, TMap<EWeaponCategory, UChildActorComponent*>& Inventory);
	UFUNCTION(BlueprintCallable, Category = "Weapon") bool GetCategoryUpgrades(EWeaponCategory Category, TArray<TSubclassOf<ABaseWeapon>>& Upgrades);
	UFUNCTION(BlueprintCallable, Category = "Weapon") int32 GetCategoryUpgradeSize(EWeaponCategory Category);
	UFUNCTION(BlueprintCallable, Category = "Weapon") FUpgradeCardInfo GetWeaponInfo(EWeaponCategory Category, int32 UpgradeLevel);
	UFUNCTION(BlueprintCallable, Category = "Weapon") void GetAvailableEvolutions(const TArray<FOwnedWeapon>& OwnedWeapons, const TMap<EPlayerStats, int32>& OwnedStats, TArray<FUpgradeOffer>& OutEvolutions);
	UFUNCTION(BlueprintCallable, Category = "Weapon") int32 GetCategoryEvolutionIndex(EWeaponCategory Category);
	UFUNCTION(BlueprintCallable, Category = "Weapon") EWeaponUpgradeStatus SetWeapon(FWeaponCreateParams& Params, TMap<EWeaponCategory, UChildActorComponent*>& Inventory);
};