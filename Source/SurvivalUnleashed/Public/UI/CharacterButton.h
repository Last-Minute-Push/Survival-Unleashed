// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterButton.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCButtonClick);

UCLASS()
class SURVIVALUNLEASHED_API UCharacterButton : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* Price;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* Description;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* CharacterName;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UWidgetSwitcher* Switcher;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UImage* CharacterImage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) USoundBase* ButtonHoverSFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) USoundBase* ButtonClickedSFX;
	USoundBase* CurrentButtonClicked;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UButton* Button;
	UPROPERTY(BlueprintAssignable) FOnCButtonClick CodeButtonClick;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText PriceInfo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText CharacterInfo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DescriptionInfo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FSlateFontInfo FInfo;

	UFUNCTION(BlueprintCallable) void HandleBackgroundClick();
	UFUNCTION() void HandleHoverSFX();
	UFUNCTION() void SetPriceText(FText text);
	UFUNCTION() void SetCharacterNameText(FText text);
	UFUNCTION() void SetDescriptionText(FText text);
	UFUNCTION() void SetButtonFont(FSlateFontInfo FontInfo);
	UFUNCTION() void SetButtonClickSFX(USoundBase* SFX);

	bool Locked = true;// SET FALSE TO UNLOCK CHARCATER
	void LockCheck(); //USE AFTER UNLOCKING TO UNLOCK THE UI AS WELL

	virtual void NativePreConstruct() override;

	virtual void NativeOnInitialized() override;
};
