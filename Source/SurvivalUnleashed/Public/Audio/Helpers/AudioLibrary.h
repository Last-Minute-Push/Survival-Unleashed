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
	
	//Test Music
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Music) USoundBase* BGM_TestSong1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Music) USoundBase* BGM_TestSong2;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Music) USoundBase* BGM_TestSong3;

	//Menu Music
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Music) USoundBase* BGM_MainMenuSong;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Music) USoundBase* BGM_ResultsMenuSong;

	//Level Music
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Music) USoundBase* BGM_GameplaySong;
	

	// ---SOUNDS---

	//Test Sounds
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound) USoundBase* SFX_TestSound1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound) USoundBase* SFX_TestSound2;

	// Button SFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound) USoundBase* SFX_ButtonHover;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound) USoundBase* SFX_NeutralButtonClicked;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound) USoundBase* SFX_BackButtonClicked;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound) USoundBase* SFX_UpgradeButtonClicked;

	//Hurt SFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound) USoundBase* SFX_PlayerHurt;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound) USoundBase* SFX_EnemyHurt;

	//Weapon SFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound) USoundBase* SFX_ProjectileShoot;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound) USoundBase* SFX_MeleeSwing;

	//PickUp SFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound) USoundBase* SFX_EXPPickUp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound) USoundBase* SFX_HealthPickUp;
};
