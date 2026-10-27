#pragma once

#include "CoreMinimal.h"
#include "Utility/PlayerStatsComponent.h" // EPlayerStats
#include "Weapons/WeaponDatabase.h"
#include "UpgradeOffers.generated.h"

class UWeaponData;

UENUM(BlueprintType)
enum class EUpgradeKind : uint8 { Stat, Weapon, Heal };

// One card on the level-up screen
USTRUCT(BlueprintType)
struct FUpgradeOffer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) EUpgradeKind Kind = EUpgradeKind::Heal;
	UPROPERTY(BlueprintReadOnly) EPlayerStats Stat = EPlayerStats::EPS_Health; // when Kind == Stat
	UPROPERTY(BlueprintReadOnly) EWeaponCategory Weapon;                       // when Kind == Weapon
	UPROPERTY(BlueprintReadOnly) int32 CurrentLevel = 0;                       // before picking; 0 = NEW

	bool IsNew() const { return Kind != EUpgradeKind::Heal && CurrentLevel == 0; }
};

USTRUCT(BlueprintType)
struct FOwnedWeapon
{
	GENERATED_BODY()

	EWeaponCategory Data;
	int32 Level = 1;
	int32 MaxLevel = 1;
};

// What the player has right now
struct FBuildSnapshot
{
	TMap<EPlayerStats, int32> StatLevels; // UPlayerStatsComponent::GetStatRanks()
	TArray<FOwnedWeapon> OwnedWeapons;    // from the weapon inventory (514)
	TArray<EWeaponCategory> WeaponPool;      // every weapon that can be offered (data, not hard-coded)
};

namespace UpgradeOffers
{
	constexpr int32 MaxWeapons = 3;
	constexpr int32 MaxStats = 5;

	// Up to Count distinct offers: 2 weapons + 2 stats when possible, backfilled from the other kind, then heals
	TArray<FUpgradeOffer> Generate(const FBuildSnapshot& Build, int32 Count = 4);
}
