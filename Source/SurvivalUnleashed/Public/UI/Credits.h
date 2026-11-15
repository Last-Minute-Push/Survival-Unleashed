// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Credits.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreditsClosed);
UCLASS()
class SURVIVALUNLEASHED_API UCredits : public UUserWidget
{
	GENERATED_BODY()
	
	virtual void NativeOnInitialized() override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UButtonBase* BackButton;

	UFUNCTION() void BackButtonPressed();

public:
	FOnCreditsClosed OnCreditsClose;

};
