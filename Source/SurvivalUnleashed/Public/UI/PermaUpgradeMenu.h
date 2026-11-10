// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PermaUpgradeMenu.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUpgradeClosed);


UENUM(BlueprintType)
enum class EPlayerStats : uint8
{
	EPS_Attack,
	EPS_Health,
	EPS_Defense,
	EPS_WeaponAttack,
	EPS_AttackSpeed,
	EPS_Reflect,
	EPS_DashCooldown,
	EPS_DashLength,
	EPS_Regen,
	EPS_LifeSteal,
	EPS_EXPGain,
	EPS_Evasion,
	EPS_PickupRange,
	EPS_ProjectileAmount,
	EPS_ProjectileSpeed,
	EPS_HealAmount,
	EPS_Count UMETA(Hidden)
};


UCLASS()
class SURVIVALUNLEASHED_API UPermaUpgradeMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void BackPressed();
	//Back Button
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UButtonBase* BackUpgradeButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* AttackButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* HealthButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* DefenseButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* WeaponAttackButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* AttackSpeedButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* ReflectButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* DashCooldownButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* DashLengthButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* RegenButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* LifeStealButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* EXPGainButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* EvasionButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* PickUpRangeButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* ProjectileAmountButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* ProjectileSpeedButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* HealAmountButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* BarbaricButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* ModernButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeButton* IntelligenceButton;

	UPROPERTY()
	int PointAmount = 0;

	UPROPERTY()
	TMap<EPlayerStats, int> UpgradeQueue; //what the player needs to upgrade when they start

	UPROPERTY()
	TMap<EPlayerStats, int> UpgradeCosts; //what each upgrade cost

	
public:
	

	UPROPERTY(BlueprintAssignable, Category = "Upgrades")
	FOnUpgradeClosed OnUpgradeClose;
	

};
