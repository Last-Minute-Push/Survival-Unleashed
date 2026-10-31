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

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) UOptionsSliderBase* MasterSlider;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) UOptionsSliderBase* MusicSlider;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) UOptionsSliderBase* SoundSlider;
};