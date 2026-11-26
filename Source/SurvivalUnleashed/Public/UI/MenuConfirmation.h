// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/HUDBase.h"
#include "MenuConfirmation.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuClosed);
UCLASS()
class SURVIVALUNLEASHED_API UMenuConfirmation : public UHUDBase
{
	GENERATED_BODY()

	virtual void NativeOnInitialized() override;

protected:

	UPROPERTY(meta = (BindWidget)) class UButtonBase* YesButton;
	UPROPERTY(meta = (BindWidget)) class UButtonBase* NoButton;

public:
	UFUNCTION() void NoMenuButtonPressed();

	UMenuConfirmation();

	FOnMenuClosed OnMenuClosed;

};
