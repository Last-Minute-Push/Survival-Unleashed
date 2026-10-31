// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"
#include "Audio/Helpers/AudioLibrary.h"

#include "AudioManagerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAudioManagerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SURVIVALUNLEASHED_API IAudioManagerInterface
{
	GENERATED_BODY()

public:

	// General
	virtual UAudioLibrary* GetLibrary() const = 0;
	virtual float GetMasterVolume() const = 0;
	virtual float GetMusicVolume(bool bRaw = false) const = 0;
	virtual float GetSoundVolume(bool bRaw = false) const = 0;
	virtual void SetMasterVolume(float Volume) = 0;
	virtual void SetMusicVolume(float Volume) = 0;
	virtual void SetSoundVolume(float Volume) = 0;


	// Music
	UFUNCTION() virtual uint64 PlayMusic(USoundBase* Music, const float NewFadeIn = 0, const float OldFadeOut = 0, USoundConcurrency* ConcurrencySettings = nullptr) = 0;
	UFUNCTION() virtual void SetMusicPause(const bool bPaused) = 0;
	UFUNCTION() virtual uint64 StackMusic(USoundBase* Music, const float FadeIn = 0, USoundConcurrency* ConcurrencySettings = nullptr) = 0;
	UFUNCTION() virtual void DestackMusic(const float FadeOut = 0.f, const bool bSkipResume = false) = 0;
	UFUNCTION() virtual void ResetStack() = 0;
	UFUNCTION() virtual void StopMusic(const float FadeOut = 0) = 0;

	// Sounds
	// Cheapest option: returns no ID and can't be paused or stopped. Use for hits, pickups, UI clicks.
	UFUNCTION() virtual void PlaySoundOneShot(USoundBase * Sound, USoundConcurrency * ConcurrencySettings = nullptr) = 0;

	UFUNCTION() virtual uint64 PlaySound(USoundBase* Sound, USoundConcurrency* ConcurrencySettings = nullptr) = 0;
	UFUNCTION() virtual uint64 PlaySoundInterrupt(USoundBase* Sound, USoundConcurrency* ConcurrencySettings = nullptr) = 0;
	UFUNCTION() virtual void SetSoundPause(USoundBase const* Sound, const bool bPaused) = 0;
	UFUNCTION() virtual void SetOneSoundPause(const uint64 AudioComponentID, const bool bPaused) = 0;
	UFUNCTION() virtual void SetAllSoundPause(const bool bPaused) = 0;
	UFUNCTION() virtual void StopSound(USoundBase const* Sound) = 0;
	UFUNCTION() virtual void StopOneSound(const uint64 AudioComponentID) = 0;
	UFUNCTION() virtual void StopAllSound() = 0;
};
