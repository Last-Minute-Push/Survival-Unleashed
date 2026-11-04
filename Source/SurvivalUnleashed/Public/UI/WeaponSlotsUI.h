// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Weapons/WeaponDatabase.h"
#include "WeaponSlotsUI.generated.h"

/**
 * 
 */

UCLASS()
class SURVIVALUNLEASHED_API UWeaponSlotsUI : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UWidgetSwitcher* WeaponIconSwitcherOne;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* SlotOneLevel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UWidgetSwitcher* WeaponIconSwitcherTwo;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* SlotTwoLevel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UWidgetSwitcher* WeaponIconSwitcherThree;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* SlotThreeLevel;

public:
	
	void SetWeaponSlot(EWeaponCategory Category, int WeaponSlot);

	void SetWeaponSlotLevel(int placeholder);

	int SetWeaponImageIndex(EWeaponCategory Category);
};
