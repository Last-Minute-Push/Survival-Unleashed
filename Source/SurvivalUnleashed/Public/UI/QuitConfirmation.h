// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/HUDBase.h"
#include "QuitConfirmation.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuitClosed);
UCLASS()
class SURVIVALUNLEASHED_API UQuitConfirmation : public UHUDBase
{
	GENERATED_BODY()

	UQuitConfirmation();

	virtual void NativeOnInitialized() override;
	
protected:

	UPROPERTY(meta = (BindWidget)) class UButtonBase* YesButton;
	UPROPERTY(meta = (BindWidget)) class UButtonBase* NoButton;
public:
	UFUNCTION() void NoButtonPressed();

	FOnQuitClosed OnQuitClosed;
};
