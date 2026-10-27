// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "ButtonBase.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnButtonClick);
UCLASS()
class SURVIVALUNLEASHED_API UButtonBase : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativePreConstruct() override;

	virtual void NativeOnInitialized() override;

protected:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* Info;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) USoundBase* ButtonHoverSFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) USoundBase* ButtonClickedSFX;
	USoundBase* CurrentButtonClicked;

public:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UButton* TheButton;
	UPROPERTY(BlueprintAssignable) FOnButtonClick CodeButtonClick;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText CodeInfo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FSlateFontInfo FInfo;

	UFUNCTION(BlueprintCallable) void HandleBackgroundClick();
	UFUNCTION() void HandleHoverSFX();
	UFUNCTION() void SetButtonText(FText text);
	UFUNCTION() void SetButtonFont(FSlateFontInfo FontInfo);
	UFUNCTION() void SetButtonClickSFX(USoundBase* SFX);

	
	
};
