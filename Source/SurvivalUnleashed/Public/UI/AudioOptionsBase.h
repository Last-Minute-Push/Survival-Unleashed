// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AudioOptionsBase.generated.h"

class UOptionsSliderBase;

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UAudioOptionsBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UOptionsSliderBase> MasterSlider;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UOptionsSliderBase> MusicSlider;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UOptionsSliderBase> SoundSlider;

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
};