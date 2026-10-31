// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AudioOptionsBase.h"
#include "UI/OptionsSliderBase.h"
#include "Audio/Interfaces/AudioManagerInterface.h"

void UAudioOptionsBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{
		MasterSlider->OnValueChanged.AddDynamic(I, &IAudioManagerInterface::SetMasterVolume);
		MusicSlider->OnValueChanged.AddDynamic(I, &IAudioManagerInterface::SetMusicVolume);
		SoundSlider->OnValueChanged.AddDynamic(I, &IAudioManagerInterface::SetSoundVolume);
	}
}

void UAudioOptionsBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{
		MasterSlider->SetSliderValue(I->GetMasterVolume());
		MusicSlider->SetSliderValue(I->GetMusicVolume(true));
		SoundSlider->SetSliderValue(I->GetSoundVolume(true));
	}
}