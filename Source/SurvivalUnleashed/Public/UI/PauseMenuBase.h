// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Framework/UGI_SUGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UI/OptionsMenuBase.h"
#include "PauseMenuBase.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UPauseMenuBase : public UUserWidget
{
	GENERATED_BODY()

	
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

	UPROPERTY()
	UOptionsMenuBase* OptionHud;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> OptionHUDClass;

	UFUNCTION()
	void UnHidePauseButtons();
	
};
