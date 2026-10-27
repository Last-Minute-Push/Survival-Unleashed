// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ButtonBase.h"
#include "Weapons/WeaponDatabase.h"
#include "WeaponUpgradeButton.generated.h"


/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeButtonClick, EWeaponCategory, Category);

UCLASS()
class SURVIVALUNLEASHED_API UWeaponUpgradeButton : public UButtonBase
{
	GENERATED_BODY()
	
	virtual void NativeOnInitialized() override;

	void HandleUpgradeBackgroundClick();

	UPROPERTY(BlueprintAssignable) FOnUpgradeButtonClick UpgradeButtonClick;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Upgrade Type") EWeaponCategory Type;
};
