// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuitConfirmation.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuitClosed);
UCLASS()
class SURVIVALUNLEASHED_API UQuitConfirmation : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeOnInitialized() override;


protected:

	UPROPERTY(meta = (BindWidget)) class UButtonBase* YesButton;
	UPROPERTY(meta = (BindWidget)) class UButtonBase* NoButton;

	UFUNCTION() void NoButtonPressed();
public:
	FOnQuitClosed OnQuitClosed;
};
