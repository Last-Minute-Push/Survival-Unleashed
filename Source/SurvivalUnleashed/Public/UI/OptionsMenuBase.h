// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/HUDBase.h"
#include "OptionsMenuBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOptionsClosed);
/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UOptionsMenuBase : public UHUDBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void BackPressed();

	UFUNCTION()
	void AudioPressed();

	UFUNCTION()
	void GraphicsPressed();

	UFUNCTION()
	void ControlsPressed();


	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButtonBase* AudioButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButtonBase* GraphicsButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButtonBase* ControlsButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UWidgetSwitcher* OptionsSwitcher;

public:
	UOptionsMenuBase();

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButtonBase* BackButton;

	// Fires after Options closes, so whichever menu opened it can show itself again
	UPROPERTY(BlueprintAssignable, Category = "Options")
	FOnOptionsClosed OnClosed;

	void Back();
};
