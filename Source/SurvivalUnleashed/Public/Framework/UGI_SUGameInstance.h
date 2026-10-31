// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Engine/GameInstance.h"
#include "Components/AudioComponent.h"
#include "Audio/Interfaces/AudioManagerInterface.h"
#include "Core/RunResults.h"
#include "Weapons/WeaponData.h" // Focus enum

#include "UGI_SUGameInstance.generated.h"

// Activates the audio test
#define AUDIO_MANAGER_TEST 0

/**
 *
 */
UCLASS()
class SURVIVALUNLEASHED_API UUGI_SUGameInstance : public UGameInstance, public IAudioManagerInterface
{
	GENERATED_BODY()

protected:

	// - AudioManager -
	FTimerHandle MusicFadeInTimer;
	FTimerHandle DestackResumeTimer;

	UPROPERTY(Transient) TObjectPtr<UAudioComponent> CurrentMusic;
	UPROPERTY(Transient) TArray<TObjectPtr<UAudioComponent>> MusicStack;     // paused songs, last in first out
	UPROPERTY(Transient) TArray<TObjectPtr<UAudioComponent>> FadingOutMusic; // old songs finishing their fade-out
	UPROPERTY(Transient) TArray<TObjectPtr<UAudioComponent>> ActiveSounds;   // tracked sounds (can be paused/stopped)


	UPROPERTY(EditDefaultsOnly, Category = Audio) TSubclassOf<UAudioLibrary> AudioLibraryClass; //! SET IN EDITOR (BP_AudioLibrary)
	
	UAudioComponent* CreateAudio(USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier,
		USoundConcurrency* ConcurrencySettings, bool bPersistAcrossLevels);

	void ApplyMusicVolume();
	void ApplySoundVolume();


	// - Weapon Database -
	UPROPERTY(EditDefaultsOnly, Category = "Weapons") UWeaponDatabase* Database;


	// - Level Handling -
	int32 CurrentLevel = 0;
	UPROPERTY(EditDefaultsOnly, Category = Levels) int32 FirstLevelIndex = 1;
	UPROPERTY(EditDefaultsOnly, Category = Levels) int32 MainMenuIndex = 0;
	UPROPERTY(EditDefaultsOnly, Category = Levels) TArray<FName> GameLevels; //! SET IN EDITOR!


	// - Save / Run Results -
	UPROPERTY(EditDefaultsOnly, Category = Save) FString SaveSlotName = TEXT("SU_Progress");

	static constexpr int32 SaveUserIndex = 0;
	UPROPERTY() TObjectPtr<class USUSaveGame> SaveData;
	UPROPERTY() FRunResults LastRunResults;

	void LoadOrCreateSave();

public:

	UUGI_SUGameInstance();

	virtual void Init() override;

	//----------------------------------------------------------------------//
    // Level Handling
    //----------------------------------------------------------------------//
	void LoadLevel(const int32 LevelIndex);
	void LoadLevelSafe(const int32 LevelIndex);
	UFUNCTION() inline void LoadFirstLevel() { LoadLevelSafe(FirstLevelIndex); }
	UFUNCTION() void ReloadLevel();
	UFUNCTION() void QuitToMenu();
	UFUNCTION() void QuitTheGame();

	//----------------------------------------------------------------------//
    // Weapon Database
    //----------------------------------------------------------------------//
	UFUNCTION() inline UWeaponDatabase* GetWeaponDatabase() const { return Database; }

	//----------------------------------------------------------------------//
	// Save / Run Results
	//----------------------------------------------------------------------//
	// Writes SaveData to disk. Call after anything in SaveData changes.
	UFUNCTION(BlueprintCallable, Category = Save) void SaveProgress();
	// Called by the GameMode when a run ends. Fills in points and personal best, then saves.
	void RecordRunResults(const FRunResults& Results);

	UFUNCTION(BlueprintPure, Category = Save) USUSaveGame* GetSaveData() const { return SaveData; }
	UFUNCTION(BlueprintPure, Category = "Run Results") FRunResults GetLastRunResults() const { return LastRunResults; }

	//----------------------------------------------------------------------//
	// Permanent Upgrades
	// Stores and saves only. Costs, rank caps, and focus rules live in the stat component.
	//----------------------------------------------------------------------//
	UFUNCTION(BlueprintPure, Category = "Permanent Upgrades") int32 GetUpgradePoints() const;
	UFUNCTION(BlueprintPure, Category = "Permanent Upgrades") int32 GetPermanentRank(FName Stat) const;
	UFUNCTION(BlueprintPure, Category = "Permanent Upgrades") int32 GetTotalPermanentRanks() const;
	UFUNCTION(BlueprintPure, Category = "Permanent Upgrades") int32 GetPermanentFocus(Focus FocusStat) const;
	UFUNCTION(BlueprintPure, Category = "Permanent Upgrades") int32 GetFocusPointsSpent() const;

	// Buys one rank if the player can afford it and it's below MaxRank. Saves on success.
	UFUNCTION(BlueprintCallable, Category = "Permanent Upgrades") bool TryBuyRank(FName Stat, int32 Cost, int32 MaxRank);
	// Spends one permanent focus point if fewer than FocusPointsEarned are spent. Saves on success.
	UFUNCTION(BlueprintCallable, Category = "Permanent Upgrades") bool TrySpendFocusPoint(Focus FocusStat, int32 FocusPointsEarned);
	// Returns every spent point and clears all ranks and focus points. Saves.
	UFUNCTION(BlueprintCallable, Category = "Permanent Upgrades") void RefundAllRanks();

#if AUDIO_MANAGER_TEST
	void AudioManagerTest();
#endif
	//----------------------------------------------------------------------//
	// IAudioManagerInterface
	//----------------------------------------------------------------------//
	// - General -
	// The library is read-only data, so the Blueprint's class defaults are used directly
	virtual UAudioLibrary* GetLibrary() const override
	{
		return AudioLibraryClass ? AudioLibraryClass->GetDefaultObject<UAudioLibrary>() : nullptr;
	}

	virtual float GetMasterVolume() const;
	virtual float GetMusicVolume(bool bRaw = false) const;
	virtual float GetSoundVolume(bool bRaw = false) const;

	UFUNCTION() virtual void SetMasterVolume(float Volume);
	UFUNCTION() virtual void SetMusicVolume(float Volume);
	UFUNCTION() virtual void SetSoundVolume(float Volume);



	// - Music -

	// We play the new song
	UFUNCTION() virtual uint64 PlayMusic(USoundBase* Music, const float NewFadeIn = 0, const float OldFadeOut = 0, USoundConcurrency* ConcurrencySettings = nullptr) override;

	// We pause the current song
	UFUNCTION() virtual void SetMusicPause(const bool bPaused) override;
	// We pause the current song, place it on top of the stack of CDs, and play the new song
	UFUNCTION() virtual uint64 StackMusic(USoundBase* Music, const float FadeIn = 0, USoundConcurrency* ConcurrencySettings = nullptr) override;
	// We stop the current song and remove the top song from the music stack of CDs to play it
	UFUNCTION() virtual void DestackMusic(const float FadeOut = 0.f, const bool bSkipResume = false) override;
	// We clear the stack of CDs
	UFUNCTION() virtual void ResetStack() override;
	// We stop and destroy the current song and clean up the entire music stack of CDs
	UFUNCTION() virtual void StopMusic(const float FadeOut = 0) override;

	// - Sounds -

	// Fire and forget: no component, no ID. Use for anything you'll never pause or stop.
	UFUNCTION() virtual void PlaySoundOneShot(USoundBase* Sound, USoundConcurrency* ConcurrencySettings = nullptr) override;

	// We play a sound (regardless of audio spam)
	UFUNCTION() virtual uint64 PlaySound(USoundBase* Sound, USoundConcurrency* ConcurrencySettings = nullptr) override;
	// We stop all instances of our sound to hear our sound play only once (handles audio spam)
	UFUNCTION() virtual uint64 PlaySoundInterrupt(USoundBase* Sound, USoundConcurrency* ConcurrencySettings = nullptr) override;
	// We pause or resume all instances of our same sound
	UFUNCTION() virtual void SetSoundPause(USoundBase const* Sound, const bool bPaused) override;
	// We pause or resume only a specific sound linked to the AudioComponentID
	UFUNCTION() virtual void SetOneSoundPause(const uint64 AudioComponentID, const bool bPaused) override;
	// We pause or resume all sounds
	UFUNCTION() virtual void SetAllSoundPause(const bool bPaused) override;
	// We stop and destroy all instances of our sound
	UFUNCTION() virtual void StopSound(USoundBase const* Sound) override;
	// We stop and destroy only a specific sound with an AudioComponentID
	UFUNCTION() virtual void StopOneSound(const uint64 AudioComponentID) override;
	// We stop and destroy all sounds
	UFUNCTION() virtual void StopAllSound() override;
};
