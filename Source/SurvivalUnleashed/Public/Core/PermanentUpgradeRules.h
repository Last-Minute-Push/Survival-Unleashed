// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utility/PlayerStatsComponent.h" // EPlayerStats
#include "PermanentUpgradeRules.generated.h"

/**
 * The rules for permanent upgrades, as data: what each rank costs, how many ranks a stat can have,
 * how many ranks earn a permanent focus point, and how much one rank adds to each stat.
 * The game instance stores and saves the ranks; this asset says what they cost and what they do.
 * It's an asset, not part of the stat component, because the upgrade screen runs on the main menu, where there's no player.
 */
UCLASS(BlueprintType)
class SURVIVALUNLEASHED_API UPermanentUpgradeRules : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPermanentUpgradeRules();

	// Cost of rank 1, 2, 3, ... The number of entries is the rank cap.
	UPROPERTY(EditDefaultsOnly, Category = "Costs")
	TArray<int32> RankCosts = { 5, 10, 15, 20, 25 };

	// Every this many ranks bought, across all stats, earns one permanent focus point. GDD: 15.
	UPROPERTY(EditDefaultsOnly, Category = "Focus", meta = (ClampMin = "1"))
	int32 RanksPerFocusPoint = 15;

	// What one rank adds, in the units of that stat's getter on the stat component (see the constructor)
	UPROPERTY(EditDefaultsOnly, Category = "Bonuses")
	TMap<EPlayerStats, float> BonusPerRank;

	int32 GetMaxRank() const { return RankCosts.Num(); }

	// The cost of the next rank for a stat that has CurrentRank, or -1 if it's maxed
	UFUNCTION(BlueprintPure, Category = "Permanent Upgrades")
	int32 GetNextRankCost(int32 CurrentRank) const;

	UFUNCTION(BlueprintPure, Category = "Permanent Upgrades")
	float GetBonus(EPlayerStats Stat, int32 Rank) const { return BonusPerRank.FindRef(Stat) * Rank; }

	// The save's key for a stat ("EPS_Health"). Names, not numbers, so reordering the enum can't scramble old saves.
	static FName GetStatKey(EPlayerStats Stat);
};
