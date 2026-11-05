// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/HUDBase.h"
#include "Framework/UGI_SUGameInstance.h"
#include "MainMenu.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UMainMenu : public UHUDBase
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UButtonBase* PlayButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UButtonBase* UpgradeButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UButtonBase* SettingsButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UButtonBase* QuitButton;
	UPROPERTY() TObjectPtr<class UOptionsMenuBase> OptionHud;
	UPROPERTY() TObjectPtr<class UPermaUpgradeMenu> UpgradeHud;

	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void SettingsButtonPressed();

	UFUNCTION()
	void UpgradeButtonPressed();

	UFUNCTION()
	void EndMainMenuMusic();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> OptionHUDClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> UpgradeHUDClass;

	UFUNCTION()
	void UnHideMainMenuButtons();

};
