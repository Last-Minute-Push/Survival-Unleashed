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
	void Construct();

	void UpdateTimer(float TimePassed);

	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UProgressBar* HealthBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UProgressBar* EXPBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* TimerText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* EXPGainCounter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* LevelCounter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* KillCounter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* FocusCounter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* PersonalBest;

	//UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UHorizontalBox* StatBox;
	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnim)) class UWidgetAnimation* Fading;

	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnim)) class UWidgetAnimation* FadingHeal;

	FText GetRankName(uint8 Stats);

public:

	UFUNCTION(BlueprintCallable) void SetHealth(float percent);

	UFUNCTION(BlueprintCallable) void SetExperience(float ratio, float Max);

	UFUNCTION(BlueprintCallable) void SetLevelAndExp(float Level, float EXP);

	UFUNCTION(BlueprintCallable) void SetPersonalBestTime(FText Time);

	UFUNCTION(BlueprintCallable) void PlayHurtFeedback();

	UFUNCTION(BlueprintCallable) void PlayHealFeedback();

	UFUNCTION(BlueprintCallable) void SetStatsText(uint8 ChosenStats, int32 Ranks);

	FVector EndPoint;

	TArray<FText> RankArray;
	TArray<uint32> RankIndex;


	float Timer;

	bool Starting = true;

	bool isPaused;
	
	FText GetCurrentTimer();
};
