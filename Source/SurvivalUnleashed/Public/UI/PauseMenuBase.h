// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/HUDBase.h"
#include "Framework/UGI_SUGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UI/OptionsMenuBase.h"
#include "PauseMenuBase.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UPauseMenuBase : public UHUDBase
{
	GENERATED_BODY()

	UPauseMenuBase();
protected: 
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UButtonBase* ContinueButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UButtonBase* OptionsButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UButtonBase* QuitButton;
	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnim)) class UWidgetAnimation* BlurEffect;

	UFUNCTION()
	void ResumeGame();

	UFUNCTION()
	void OptionsButtonPressed();

	UFUNCTION()
	void MenuButtonPressed();

	UPROPERTY()
	UOptionsMenuBase* OptionHud;

	UPROPERTY() TObjectPtr<class UMenuConfirmation> MenuHud;

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> OptionHUDClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> MenuHUDClass;

	UFUNCTION()
	void UnHidePauseButtons();
	
};
