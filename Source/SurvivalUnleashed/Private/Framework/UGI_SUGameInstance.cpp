// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UGI_SUGameInstance.h"
#include "Sound/SoundBase.h"
#include "AudioDevice.h"
#include "Core/SUSaveGame.h"
#include "Kismet/GameplayStatics.h"

#ifndef UINT64_MAX
#define UINT64_MAX 0xffffffffffffffffui64
#endif // !UINT64_MAX

DEFINE_LOG_CATEGORY_STATIC(LogGameInstance, Log, All);

UUGI_SUGameInstance::UUGI_SUGameInstance() : AudioLibraryClass(UAudioLibrary::StaticClass()) {}

void UUGI_SUGameInstance::Init()
{
	Super::Init();
	LoadOrCreateSave();
}


//----------------------------------------------------------------------//
// Level Handling
//----------------------------------------------------------------------//

void UUGI_SUGameInstance::LoadLevel(const int32 LevelIndex)
{
	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogGameInstance, Error, TEXT("Failed to retrieve world!"));
		return;
	}
	else if (!GEngine)
	{
		UE_LOG(LogGameInstance, Error, TEXT("GEngine is nullptr!"));
		return;
	}

	// Setting travel type and retrieving World Context
	const ETravelType TravelType = TRAVEL_Absolute;

	// Retrieving level name
	if (GameLevels.Num() == 0)
	{
		UE_LOG(LogGameInstance, Error, TEXT("No levels are present in the GameLevels array from the GameInstance!"));
		return;
	}

	FString LevelName = GameLevels[LevelIndex].ToString();

	// Verifying the level exists if it's a local file
	FURL TestURL(&WorldContext->LastURL, *LevelName, TravelType);
	if (TestURL.IsLocalInternal() && !GEngine->MakeSureMapNameIsValid(TestURL.Map))
	{
		UE_LOG(LogGameInstance, Error, TEXT("The level \"%s\" could not be found!"), *TestURL.Map);
		return;
	}
	CurrentLevel = LevelIndex;

	// Allows the client to travel to the level.
	GEngine->SetClientTravel(World, *LevelName, TravelType);
}

void UUGI_SUGameInstance::LoadLevelSafe(const int32 LevelIndex)
{
	if (GameLevels.IsValidIndex(LevelIndex)) LoadLevel(LevelIndex);
	else UE_LOG(LogGameInstance, Error, TEXT("The level index (%i) is not a valid index in the game instance!"), CurrentLevel);
}

void UUGI_SUGameInstance::ReloadLevel()
{
	LoadLevelSafe(CurrentLevel);
}

void UUGI_SUGameInstance::QuitToMenu()
{
	LoadLevelSafe(MainMenuIndex);
}

void UUGI_SUGameInstance::QuitTheGame()
{
	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogGameInstance, Error, TEXT("Failed to retrieve world!"));
		return;
	}
	else if (World->GetGameInstance() != this) // Also checks if GameInstance is nullptr. "this" can't ever be nullptr.
	{
		UE_LOG(LogGameInstance, Error, TEXT("Failed to retrieve game instance or not using same instance!"));
	}

	// Retrieve the controller and send the shutdown command to it
	for (ULocalPlayer* LP : LocalPlayers)
	{
		if (APlayerController* PC = LP->PlayerController)
		{
			PC->ConsoleCommand("quit force");
			return;
		}
	}
}

//----------------------------------------------------------------------//
// Save / Run Results
//----------------------------------------------------------------------//

void UUGI_SUGameInstance::LoadOrCreateSave()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		SaveData = Cast<USUSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));
		if (!SaveData) UE_LOG(LogGameInstance, Warning, TEXT("Save \"%s\" exists but could not be loaded. Creating a new one."), *SaveSlotName);
	}

	if (!SaveData)
	{
		SaveData = Cast<USUSaveGame>(UGameplayStatics::CreateSaveGameObject(USUSaveGame::StaticClass()));
		UE_LOG(LogGameInstance, Log, TEXT("Created new save \"%s\"."), *SaveSlotName);
		SaveProgress();
		return;
	}

	if (SaveData->SaveVersion < USUSaveGame::LatestVersion)
	{
		// Fix up older saves one version at a time
		if (SaveData->SaveVersion < 2)
		{
			// v1 had no way to buy ranks, so nothing has been spent yet
			SaveData->SpentPoints = 0;
		}
	}

	UE_LOG(LogGameInstance, Log, TEXT("Loaded save \"%s\": %d points, best time %.1f s."), *SaveSlotName, SaveData->UpgradePoints, SaveData->BestTime);
}

void UUGI_SUGameInstance::SaveProgress()
{
	if (!SaveData) return;

	if (!UGameplayStatics::SaveGameToSlot(SaveData, SaveSlotName, SaveUserIndex))
		UE_LOG(LogGameInstance, Error, TEXT("Failed to write save \"%s\"!"), *SaveSlotName);
}

void UUGI_SUGameInstance::RecordRunResults(const FRunResults& Results)
{
	LastRunResults = Results;
	if (!SaveData) return;

	LastRunResults.PreviousBestTime = SaveData->BestTime;
	LastRunResults.PreviousBestKills = SaveData->BestKills;
	LastRunResults.PreviousBestLevel = SaveData->BestLevel;
	LastRunResults.PreviousBestScore = SaveData->BestScore;
	LastRunResults.bNewPersonalBest = Results.TimeSurvived > SaveData->BestTime; // first run: BestTime is 0, so always a new best
	LastRunResults.PointsEarned = Results.CalculatePoints();

	SaveData->UpgradePoints += LastRunResults.PointsEarned;
	if (LastRunResults.bNewPersonalBest) SaveData->BestTime = Results.TimeSurvived;
	SaveData->BestKills = FMath::Max(SaveData->BestKills, Results.Kills);
	SaveData->BestLevel = FMath::Max(SaveData->BestLevel, Results.LevelReached);
	SaveData->BestScore = FMath::Max(SaveData->BestScore, Results.CalculatePoints());
	SaveProgress();
}

//----------------------------------------------------------------------//
// Permanent Upgrades
//----------------------------------------------------------------------//

int32 UUGI_SUGameInstance::GetUpgradePoints() const
{
	return SaveData ? SaveData->UpgradePoints : 0;
}

int32 UUGI_SUGameInstance::GetPermanentRank(FName Stat) const
{
	return SaveData ? SaveData->PermanentRanks.FindRef(Stat) : 0;
}

int32 UUGI_SUGameInstance::GetTotalPermanentRanks() const
{
	int32 Total = 0;
	if (SaveData)
	{
		for (const TPair<FName, int32>& Rank : SaveData->PermanentRanks)
		{
			Total += Rank.Value;
		}
	}
	return Total;
}

int32 UUGI_SUGameInstance::GetPermanentFocus(Focus FocusStat) const
{
	if (!SaveData) return 0;

	switch (FocusStat)
	{
	case Focus::Intelligence: return SaveData->FocusIntelligence;
	case Focus::Barbaric:     return SaveData->FocusBarbaric;
	case Focus::Modern:       return SaveData->FocusModern;
	default:                  return 0;
	}
}

int32 UUGI_SUGameInstance::GetFocusPointsSpent() const
{
	return SaveData ? SaveData->FocusIntelligence + SaveData->FocusBarbaric + SaveData->FocusModern : 0;
}

bool UUGI_SUGameInstance::TryBuyRank(FName Stat, int32 Cost, int32 MaxRank)
{
	if (!SaveData || Stat.IsNone() || Cost < 0) return false;

	// FindRef doesn't add an entry, so a refused purchase leaves the map untouched
	const int32 Rank = SaveData->PermanentRanks.FindRef(Stat);
	if (Rank >= MaxRank || SaveData->UpgradePoints < Cost) return false;

	SaveData->PermanentRanks.Add(Stat, Rank + 1);
	SaveData->UpgradePoints -= Cost;
	SaveData->SpentPoints += Cost;
	SaveProgress();
	return true;
}

bool UUGI_SUGameInstance::TrySpendFocusPoint(Focus FocusStat, int32 FocusPointsEarned)
{
	if (!SaveData || GetFocusPointsSpent() >= FocusPointsEarned) return false;

	switch (FocusStat)
	{
	case Focus::Intelligence: ++SaveData->FocusIntelligence; break;
	case Focus::Barbaric:     ++SaveData->FocusBarbaric;     break;
	case Focus::Modern:       ++SaveData->FocusModern;       break;
	default:                  return false;
	}

	SaveProgress();
	return true;
}

void UUGI_SUGameInstance::RefundAllRanks()
{
	if (!SaveData) return;

	SaveData->UpgradePoints += SaveData->SpentPoints;
	SaveData->SpentPoints = 0;
	SaveData->PermanentRanks.Empty();

	// Permanent focus points come from ranks, so they're refunded with them
	SaveData->FocusIntelligence = 0;
	SaveData->FocusBarbaric = 0;
	SaveData->FocusModern = 0;

	SaveProgress();
}

#if AUDIO_MANAGER_TEST
void UUGI_SUGameInstance::AudioManagerTest()
{
	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(this/*GetWorld()->GetGameInstance()*/))
	{
		float Delay = 5.f;
		float Volume = 1.f;
		float OldSongFadeOut = 1.f;
		float NewSongFadeIn = 1.f;
		FTimerHandle D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, D14, D15, D16, D17, D18, D19, D20, D21;

		UE_LOG(LogGameInstance, Log, TEXT("- START OF AUDIO MANAGER TEST -"));

		// Play Music
		I->PlayMusic(I->GetLibrary()->BGM_TestSong2, NewSongFadeIn, 0.f, Volume);
		UE_LOG(LogGameInstance, Log, TEXT("Playing song 2"));

		// Replace Music
		GetTimerManager().SetTimer(D1, [I, NewSongFadeIn, OldSongFadeOut, Volume]
			{
				I->PlayMusic(I->GetLibrary()->BGM_TestSong1, NewSongFadeIn, OldSongFadeOut, Volume);
				UE_LOG(LogGameInstance, Log, TEXT("Stopping current song and playing song 1"));
			}, Delay, false);

		// Pause and Resume
		Delay = 15.f;
		GetTimerManager().SetTimer(D2, [I]
			{
				I->SetMusicPause(true);
				UE_LOG(LogGameInstance, Log, TEXT("Paused song"));
			}, Delay, false);

		Delay = 17.f;
		GetTimerManager().SetTimer(D3, [I]
			{
				I->SetMusicPause(false);
				UE_LOG(LogGameInstance, Log, TEXT("Resumed song"));
			}, Delay, false);

		// Stacking and destacking (useful for pause menu music, for instance)
		Delay = 20.f;
		GetTimerManager().SetTimer(D4, [I, NewSongFadeIn, OldSongFadeOut, Volume]
			{
				I->StackMusic(I->GetLibrary()->BGM_TestSong2, NewSongFadeIn, Volume);
				UE_LOG(LogGameInstance, Log, TEXT("Stacking and playing song 2"));
			}, Delay, false);

		Delay = 25.f;
		GetTimerManager().SetTimer(D5, [I, NewSongFadeIn, OldSongFadeOut, Volume]
			{
				I->StackMusic(I->GetLibrary()->BGM_TestSong3, NewSongFadeIn, Volume);
				UE_LOG(LogGameInstance, Log, TEXT("Stacking and playing song 3"));
			}, Delay, false);

		Delay = 30.f;
		GetTimerManager().SetTimer(D6, [I, OldSongFadeOut]
			{
				I->DestackMusic(OldSongFadeOut);
				UE_LOG(LogGameInstance, Log, TEXT("Destacking and playing song 2"));
			}, Delay, false);

		Delay = 35.f;
		GetTimerManager().SetTimer(D7, [I, OldSongFadeOut, this]
			{
				I->DestackMusic(OldSongFadeOut);
				UE_LOG(LogGameInstance, Log, TEXT("Destacking and playing song 1"));
			}, Delay, false);


		// Stop Music
		Delay = 40.f;
		GetTimerManager().SetTimer(D8, [I, OldSongFadeOut]
			{
				I->StopMusic(OldSongFadeOut);
				UE_LOG(LogGameInstance, Log, TEXT("Stopping playback for song 1"));
			}, Delay, false);

		// Playing Sound (No interruptions)
		Delay = 45.f;
		GetTimerManager().SetTimer(D9, [I, Volume]
			{
				I->PlaySound(I->GetLibrary()->SFX_TestSound1, Volume);
				UE_LOG(LogGameInstance, Log, TEXT("Playing sound 1"));
			}, Delay, false);


		Delay = 45.5f;
		GetTimerManager().SetTimer(D10, [I, Volume]
			{
				I->PlaySound(I->GetLibrary()->SFX_TestSound1, Volume);
				UE_LOG(LogGameInstance, Log, TEXT("Repeating sound (no interruption)"));
			}, Delay, false);

		// Playing Sound (with interrutpions)
		Delay = 50.f;
		GetTimerManager().SetTimer(D11, [I, Volume]
			{
				I->PlaySound(I->GetLibrary()->SFX_TestSound1, Volume);
				UE_LOG(LogGameInstance, Log, TEXT("Playing sound 1"));
			}, Delay, false);

		Delay = 50.5f;
		GetTimerManager().SetTimer(D12, [I, Volume]
			{
				I->PlaySoundInterrupt(I->GetLibrary()->SFX_TestSound1, Volume);
				UE_LOG(LogGameInstance, Log, TEXT("Repeating sound (interrupted)"));
			}, Delay, false);

		// Pausing Sound
		Delay = 55.f;
		GetTimerManager().SetTimer(D13, [I, Volume]
			{
				I->PlaySound(I->GetLibrary()->SFX_TestSound1, Volume);
				UE_LOG(LogGameInstance, Log, TEXT("Playing sound 1"));
			}, Delay, false);

		Delay = 55.5f;
		GetTimerManager().SetTimer(D14, [I, Volume]
			{
				I->PlaySound(I->GetLibrary()->SFX_TestSound2, Volume);
				UE_LOG(LogGameInstance, Log, TEXT("Playing sound 2"));
			}, Delay, false);

		Delay = 56.f;
		GetTimerManager().SetTimer(D15, [I]
			{
				I->SetAllSoundPause(true);
				UE_LOG(LogGameInstance, Log, TEXT("Sounds paused"));
			}, Delay, false);

		Delay = 57.f;
		GetTimerManager().SetTimer(D16, [I]
			{
				I->SetAllSoundPause(false);
				UE_LOG(LogGameInstance, Log, TEXT("Sounds Resumed"));
			}, Delay, false);

		// Stopping Sound
		Delay = 60.f;
		GetTimerManager().SetTimer(D17, [I, Volume]
			{
				I->PlaySound(I->GetLibrary()->SFX_TestSound1, Volume);
				UE_LOG(LogGameInstance, Log, TEXT("Playing sound 1"));
			}, Delay, false);

		Delay = 60.5f;
		GetTimerManager().SetTimer(D18, [I, Volume]
			{
				I->PlaySound(I->GetLibrary()->SFX_TestSound2, Volume);
				UE_LOG(LogGameInstance, Log, TEXT("Playing sound 2"));
			}, Delay, false);

		Delay = 61.f;
		GetTimerManager().SetTimer(D19, [I]
			{
				I->StopAllSound();
				UE_LOG(LogGameInstance, Log, TEXT("Sounds stopped"));
			}, Delay, false);

		Delay = 62.f;
		GetTimerManager().SetTimer(D20, [I]
			{
				I->SetAllSoundPause(true);
				UE_LOG(LogGameInstance, Log, TEXT("Pause sanity"));
			}, Delay, false);

		Delay = 63.f;
		GetTimerManager().SetTimer(D21, [I]
			{
				I->StopAllSound();
				UE_LOG(LogGameInstance, Log, TEXT("Stop sanity"));
				UE_LOG(LogGameInstance, Log, TEXT("- END OF AUDIO MANAGER TEST -"));
			}, Delay, false);
	}
}
#endif

void UUGI_SUGameInstance::ApplyMusicVolume()
{
	if (IsValid(CurrentMusic))
	{
		CurrentMusic->SetVolumeMultiplier(GetMusicVolume());
	}

	MusicStack.RemoveAll([](const TObjectPtr<UAudioComponent>& Song) { return !IsValid(Song); });
	for (TObjectPtr<UAudioComponent>& Song : MusicStack)
	{
		if (IsValid(Song))
		{
			Song->SetVolumeMultiplier(GetMusicVolume()); // Master volume is already factored into the music volume
		}
	}
}

void UUGI_SUGameInstance::ApplySoundVolume()
{
	for (TObjectPtr<UAudioComponent>&Sound : ActiveSounds)
	{
		if (IsValid(Sound))
		{
			Sound->SetVolumeMultiplier(GetSoundVolume()); // Master volume is already factored into the sound volume
		}
	}
}

//----------------------------------------------------------------------//
// IAudioManagerInterface
//----------------------------------------------------------------------//

namespace
{
	// Drops components that were destroyed or have finished/stopped playing (paused ones stay active)
	void PruneFinished(TArray<TObjectPtr<UAudioComponent>>& List)
	{
		List.RemoveAll([](const TObjectPtr<UAudioComponent>& Audio) { return !IsValid(Audio) || !Audio->IsActive(); });
	}
}

UAudioComponent* UUGI_SUGameInstance::CreateAudio(USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier,
	USoundConcurrency* ConcurrencySettings, bool bPersistAcrossLevels)
{
	if (!Sound)
	{
		UE_LOG(LogGameInstance, Error, TEXT("No valid sound was provided!"));
		return nullptr;
	}

	// 2D, created stopped, destroys itself when it finishes or is stopped
	UAudioComponent* Audio = UGameplayStatics::CreateSound2D(this, Sound, VolumeMultiplier, PitchMultiplier, 0.f,
		ConcurrencySettings, bPersistAcrossLevels, /*bAutoDestroy*/ true);
	if (!Audio)
	{
		UE_LOG(LogGameInstance, Error, TEXT("Could not create audio for %s"), *Sound->GetName());
	}
	return Audio;
}

float UUGI_SUGameInstance::GetMasterVolume() const
{
	return SaveData->MasterVolume;
}

float UUGI_SUGameInstance::GetMusicVolume(bool bRaw) const
{
	return bRaw ? SaveData->MusicVolume : SaveData->MasterVolume * SaveData->MusicVolume;
}

float UUGI_SUGameInstance::GetSoundVolume(bool bRaw) const
{
	return bRaw ? SaveData->SoundVolume : SaveData->MasterVolume * SaveData->SoundVolume;
}

void UUGI_SUGameInstance::SetMasterVolume(float Volume)
{
	SaveData->MasterVolume = FMath::Clamp(Volume, 0.f, 1.f);

	ApplyMusicVolume();
	ApplySoundVolume();
}

void UUGI_SUGameInstance::SetMusicVolume(float Volume)
{
	SaveData->MusicVolume = FMath::Clamp(Volume, 0.f, 1.f);

	ApplyMusicVolume();
}

void UUGI_SUGameInstance::SetSoundVolume(float Volume)
{
	SaveData->SoundVolume = FMath::Clamp(Volume, 0.f, 1.f);

	ApplySoundVolume();
}

// - Music -

uint64 UUGI_SUGameInstance::PlayMusic(USoundBase* Music, const float NewFadeIn, const float OldFadeOut, USoundConcurrency* ConcurrencySettings)
{
	StopMusic(OldFadeOut); // the old song fades and stops itself; it no longer affects the new one

	CurrentMusic = CreateAudio(Music, GetMusicVolume(), 1.f, ConcurrencySettings, /*bPersistAcrossLevels*/ true);
	if (!CurrentMusic)
	{
		return UINT64_MAX;
	}

	if (OldFadeOut > 0.f)
	{
		// Start once the old song is gone. Weak pointer: the song may be stopped before the timer fires.
		TWeakObjectPtr<UAudioComponent> Song = CurrentMusic;
		GetTimerManager().SetTimer(MusicFadeInTimer, [Song, NewFadeIn]
			{
				if (Song.IsValid()) Song->FadeIn(NewFadeIn);
			}, OldFadeOut, false);
	}
	else
	{
		CurrentMusic->FadeIn(NewFadeIn);
	}

	return CurrentMusic->GetAudioComponentID();
}

void UUGI_SUGameInstance::SetMusicPause(const bool bPaused)
{
	if (IsValid(CurrentMusic)) CurrentMusic->SetPaused(bPaused);
	else UE_LOG(LogGameInstance, Warning, TEXT("There is no music present that can be paused now!"));
}

uint64 UUGI_SUGameInstance::StackMusic(USoundBase* Music, const float FadeIn, USoundConcurrency* ConcurrencySettings)
{
	if (IsValid(CurrentMusic))
	{
		GetTimerManager().ClearTimer(MusicFadeInTimer);
		CurrentMusic->SetPaused(true);
		MusicStack.Push(CurrentMusic);
		CurrentMusic = nullptr;
	}
	else UE_LOG(LogGameInstance, Warning, TEXT("No music to stack; playing the new song only."));

	return PlayMusic(Music, FadeIn, 0.f, ConcurrencySettings);
}

void UUGI_SUGameInstance::DestackMusic(const float FadeOut, const bool bSkipResume)
{
	StopMusic(FadeOut);

	MusicStack.RemoveAll([](const TObjectPtr<UAudioComponent>& Song) { return !IsValid(Song); });
	if (MusicStack.IsEmpty())
	{
		UE_LOG(LogGameInstance, Warning, TEXT("There is no music present in the MusicStack!"));
		return;
	}

	CurrentMusic = MusicStack.Pop();
	if (bSkipResume)
	{
		return; // stays paused until SetMusicPause(false)
	}

	if (FadeOut > 0.f)
	{
		TWeakObjectPtr<UAudioComponent> Song = CurrentMusic;
		GetTimerManager().SetTimer(DestackResumeTimer, [Song]
			{
				if (Song.IsValid()) Song->SetPaused(false);
			}, FadeOut, false);
	}
	else
	{
		CurrentMusic->SetPaused(false);
	}
}

void UUGI_SUGameInstance::ResetStack()
{
	for (UAudioComponent* Song : MusicStack)
	{
		if (IsValid(Song)) Song->Stop(); // paused songs would otherwise live forever
	}
	MusicStack.Empty();
}

void UUGI_SUGameInstance::StopMusic(const float FadeOut)
{
	// A song waiting to fade in or resume shouldn't start after being stopped
	GetTimerManager().ClearTimer(MusicFadeInTimer);
	GetTimerManager().ClearTimer(DestackResumeTimer);

	UAudioComponent* Song = CurrentMusic;
	CurrentMusic = nullptr;
	if (!IsValid(Song))
	{
		return;
	}

	if (FadeOut > 0.f)
	{
		Song->FadeOut(FadeOut, 0.f); // stops itself when the fade ends
		PruneFinished(FadingOutMusic);
		FadingOutMusic.Add(Song);    // keep it referenced until then
	}
	else
	{
		Song->Stop();
	}
}

// - Sounds -

void UUGI_SUGameInstance::PlaySoundOneShot(USoundBase* Sound, USoundConcurrency* ConcurrencySettings)
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound, GetSoundVolume(), 1.f, 0.f, ConcurrencySettings);
	}
}

uint64 UUGI_SUGameInstance::PlaySound(USoundBase* Sound, USoundConcurrency* ConcurrencySettings)
{
	PruneFinished(ActiveSounds);

	UAudioComponent* Audio = CreateAudio(Sound, GetSoundVolume(), 1.f, ConcurrencySettings, /*bPersistAcrossLevels*/ false);
	if (!Audio)
	{
		return UINT64_MAX;
	}

	ActiveSounds.Add(Audio);
	Audio->Play();
	return Audio->GetAudioComponentID();
}

uint64 UUGI_SUGameInstance::PlaySoundInterrupt(USoundBase* Sound, USoundConcurrency* ConcurrencySettings)
{
	StopSound(Sound);
	return PlaySound(Sound, ConcurrencySettings);
}

void UUGI_SUGameInstance::SetSoundPause(USoundBase const* Sound, const bool bPaused)
{
	PruneFinished(ActiveSounds);
	for (UAudioComponent* Audio : ActiveSounds)
	{
		if (Audio->Sound.Get() == Sound) Audio->SetPaused(bPaused);
	}
}

void UUGI_SUGameInstance::SetOneSoundPause(const uint64 AudioComponentID, const bool bPaused)
{
	PruneFinished(ActiveSounds);
	for (UAudioComponent* Audio : ActiveSounds)
	{
		if (Audio->GetAudioComponentID() == AudioComponentID)
		{
			Audio->SetPaused(bPaused);
			return;
		}
	}
	UE_LOG(LogGameInstance, Warning, TEXT("Cannot pause or resume a sound that is not currently playing!"));
}

void UUGI_SUGameInstance::SetAllSoundPause(const bool bPaused)
{
	PruneFinished(ActiveSounds);
	for (UAudioComponent* Audio : ActiveSounds)
	{
		Audio->SetPaused(bPaused);
	}
}

void UUGI_SUGameInstance::StopSound(USoundBase const* Sound)
{
	for (UAudioComponent* Audio : ActiveSounds)
	{
		if (IsValid(Audio) && Audio->Sound.Get() == Sound) Audio->Stop();
	}
	PruneFinished(ActiveSounds); // removal happens after the loop, never during it
}

void UUGI_SUGameInstance::StopOneSound(const uint64 AudioComponentID)
{
	for (UAudioComponent* Audio : ActiveSounds)
	{
		if (IsValid(Audio) && Audio->GetAudioComponentID() == AudioComponentID)
		{
			Audio->Stop();
			break;
		}
	}
	PruneFinished(ActiveSounds);
}

void UUGI_SUGameInstance::StopAllSound()
{
	for (UAudioComponent* Audio : ActiveSounds)
	{
		if (IsValid(Audio)) Audio->Stop();
	}
	ActiveSounds.Empty();
}