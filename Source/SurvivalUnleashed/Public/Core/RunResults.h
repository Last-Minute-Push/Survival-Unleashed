#pragma once

#include "CoreMinimal.h"
#include "RunResults.generated.h"

/** Summary of one finished run. Filled by the GameMode, held by the GameInstance, read by the results screen. */
USTRUCT(BlueprintType)
struct FRunResults
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Run Results")
	float TimeSurvived = 0.f; // seconds

	UPROPERTY(BlueprintReadOnly, Category = "Run Results")
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Run Results")
	int32 LevelReached = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Run Results")
	int32 BossesKilled = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Run Results")
	int32 ElitesKilled = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Run Results")
	bool bVictory = false; // C tier: Gaia's Avatar defeated

	// Filled in by the GameInstance when the run is recorded
	UPROPERTY(BlueprintReadOnly, Category = "Run Results")
	int32 PointsEarned = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Run Results")
	float PreviousBestTime = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Run Results")
	float PreviousBestKills = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Run Results")
	float PreviousBestLevel = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Run Results")
	float PreviousBestScore = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Run Results")
	bool bNewPersonalBest = false;

	/** floor(T/30) + floor(Kills/50) + 3 per Elite + 15 per boss (+50 victory) */
	int32 CalculatePoints() const
	{
		return FMath::FloorToInt(TimeSurvived / 30.f)
			+ Kills / 50
			+ ElitesKilled * 3
			+ BossesKilled * 15
			+ (bVictory ? 50 : 0);
	}
};