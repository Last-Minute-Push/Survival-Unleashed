// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/HUDBase.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UpgradeButton.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgrade, int, Rank);
UCLASS()
class SURVIVALUNLEASHED_API UUpgradeButton : public UHUDBase
{
	GENERATED_BODY()

	virtual void NativePreConstruct() override;

	virtual void NativeOnInitialized() override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UButtonBase* TextButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UHorizontalBox* PipBoxHolder;
	UPROPERTY(EditDefaultsOnly) USoundBase* UpgradeSound;

	int currentRank;
public:
	UUpgradeButton();

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText UpgradeText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FSlateFontInfo UpgradeFontInfo;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) FSlateColor PipUpgradeColor;

	void SetPipRank(int Rank);

	UFUNCTION()
	void SetPipColors(int Index);

	UFUNCTION()
	void HandleUpgradeButtonClick();

	FOnUpgrade OnUpgradeApproved;

	int CurrentRank;
};
