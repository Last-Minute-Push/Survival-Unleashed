// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "UObject/NoExportTypes.h"

#include "AudioLibrary.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SURVIVALUNLEASHED_API UAudioLibrary : public UObject
{
	GENERATED_BODY()

public:
	//---MUSIC---
	
	// Test Music
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Music|Test") USoundBase* BGM_TestSong1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Music|Test") USoundBase* BGM_TestSong2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Music|Test") USoundBase* BGM_TestSong3;

	// Menu Music
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Music|Menu") USoundBase* BGM_MainMenuSong;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Music|Menu") USoundBase* BGM_ResultsMenuSong;

	// Level Music
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Music|Gameplay") USoundBase* BGM_GameplaySong;
	

	// ---SOUNDS---

	// Test Sounds
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Test") USoundBase* SFX_TestSound1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Test") USoundBase* SFX_TestSound2;

	// Player SFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Player") USoundBase* SFX_PlayerDash;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Player") USoundBase* SFX_PlayerHurt;

	// Button SFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Button") USoundBase* SFX_ButtonHover;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Button") USoundBase* SFX_NeutralButtonClicked;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Button") USoundBase* SFX_BackButtonClicked;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Button") USoundBase* SFX_UpgradeButtonClicked;

	// Enemy SFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Enemy") USoundBase* SFX_EnemyHurt;

	// Weapon SFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Weapon") USoundBase* SFX_ProjectileShoot;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Weapon") USoundBase* SFX_MeleeSwing;

	// PickUp SFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Pickup") USoundBase* SFX_EXPPickUp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Pickup") USoundBase* SFX_HealthPickUp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Pickup") USoundBase* SFX_WeaponPickUp;
};
