// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PermaUpgradeMenu.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUpgradeClosed);
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

public:
	

	UPROPERTY(BlueprintAssignable, Category = "Upgrades")
	FOnUpgradeClosed OnUpgradeClose;
	

};
