// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/HUDBase.h"
#include "Utility/PlayerStatsComponent.h"
#include "PlayerHUDBase.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UPlayerHUDBase : public UHUDBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	void UpdateTimer(float TimePassed);

	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	//---PLAYER HEALTH---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UProgressBar* HealthBar;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* HealthCounter;
	//---BOSS HUD---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UProgressBar* BossHealthBar;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* BossHealthCounter;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* BossNameText;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UHorizontalBox* BossHUDHolder;
	//---EXP---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UProgressBar* EXPBar;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* EXPGainCounter;
	//---TIMER---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* TimerText;
	//---LEVEL---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* LevelCounter;
	//---KILLS---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* KillCounter;
	//---FOCUS---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* FocusCounter;
	//---PERSONAL BEST---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* PersonalBest;
	//---STATS---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UWeaponSlotsUI* WeaponSlots;

	//UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UHorizontalBox* StatBox;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UVerticalBox* StatsBox;
	//---HUD ANIMATIONS---
	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnim)) class UWidgetAnimation* Fading;
	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnim)) class UWidgetAnimation* FadingHeal;
	//---GETRANKNAME: Get the according index in the widget switcher based on weapon category---
	FText GetRankName(EPlayerStats Stats);

public:
	//---SETHEALTH: Set the Health Progress Bar---
	UFUNCTION(BlueprintCallable) void SetHealth(float percent);

	//---SETTEXTHEALTH:Set the Health Counter compared with max and current---
	UFUNCTION(BlueprintCallable) void SetTextHealth(float current, float max);

	//---SETEXPERIENCE: Sets the Experience Progress Bar---
	UFUNCTION(BlueprintCallable) void SetExperience(float ratio, float Max);

	//---SETLEVELANDEXP: Sets both the Level and EXP Counter---
	UFUNCTION(BlueprintCallable) void SetLevelAndExp(float Level, float EXP);

	//---SETPERSONALBESTTIME: Sets the personal Best Time Text
	UFUNCTION(BlueprintCallable) void SetPersonalBestTime(FText Time);

	//---PLAYHURTFEEDBACK: Plays the Fading Animation---
	UFUNCTION(BlueprintCallable) void PlayHurtFeedback();

	//---PLAYHEALFEEDBACK: Plays the Heal Animation---
	UFUNCTION(BlueprintCallable) void PlayHealFeedback();

	//---SPAWNBOSSINFO: Sets the Boss Info Visible and sets the name and health and Plays Boss Music---
	UFUNCTION(BlueprintCallable) void SpawnBossInfo(const FText& BossName, float MaxHealth);

	//---CHANGEBOSSHEALTH: Sets the Boss health Progress Bar and Counter---
	UFUNCTION(BlueprintCallable) void ChangeBossHealth(float CurrentHealth, float MaxHealth);

	//---BOSSDEATHINFO: Hides the Boss Info, and stops music and plays game music again--
	UFUNCTION(BlueprintCallable) void BossDeathInfo();

	//---SETSTATSTEXT: Sets the Stats text for each text block---
	UFUNCTION(BlueprintCallable) void SetStatsText(EPlayerStats ChosenStats, int32 Ranks);

	UFUNCTION(BlueprintCallable) void SetWeaponSlots(EWeaponCategory category, int level);

	FVector EndPoint;

	TArray<EPlayerStats> RankIndex;
	float Timer;
	bool Starting = true;
	bool isPaused;
	FText GetCurrentTimer();
};
