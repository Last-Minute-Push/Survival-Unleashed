#include "Utility/UpgradeOffers.h"
#include "Algo/RandomShuffle.h"

namespace
{
	FUpgradeOffer MakeStatOffer(EPlayerStats Stat, int32 Level)
	{
		FUpgradeOffer Offer;
		Offer.Kind = EUpgradeKind::Stat;
		Offer.Stat = Stat;
		Offer.CurrentLevel = Level;
		return Offer;
	}

	FUpgradeOffer MakeWeaponOffer(EWeaponCategory Weapon, int32 Level)
	{
		FUpgradeOffer Offer;
		Offer.Kind = EUpgradeKind::Weapon;
		Offer.Weapon = Weapon;
		Offer.CurrentLevel = Level;
		return Offer;
	}

	// Moves up to Num offers off the end of a shuffled list. Never reads past an empty list.
	void TakeUpTo(TArray<FUpgradeOffer>& From, int32 Num, TArray<FUpgradeOffer>& To)
	{
		while (Num-- > 0 && From.Num() > 0)
		{
			To.Add(From.Pop(EAllowShrinking::No));
		}
	}
}

TArray<FUpgradeOffer> UpgradeOffers::Generate(const FBuildSnapshot& Build, int32 Count)
{
	// ---- Every legal weapon offer
	TArray<FUpgradeOffer> Weapons;
	for (const FOwnedWeapon& Owned : Build.OwnedWeapons)
	{
		if (Owned.Level < Owned.MaxLevel) // max-level weapons drop out
		{
			Weapons.Add(MakeWeaponOffer(Owned.Data, Owned.Level));
		}
	}
	if (Build.OwnedWeapons.Num() < MaxWeapons) // new weapons only while a slot is free
	{
		for (EWeaponCategory Weapon : Build.WeaponPool)
		{
			const bool bOwned = Build.OwnedWeapons.ContainsByPredicate([Weapon](const FOwnedWeapon& W) { return W.Data == Weapon; });
			if (!bOwned)
			{
				Weapons.Add(MakeWeaponOffer(Weapon, 0));
			}
		}
	}

	// ---- Every legal stat offer
	TArray<FUpgradeOffer> Stats;
	const bool bStatSlotFree = Build.StatLevels.Num() < MaxStats;
	for (int32 i = 0; i < static_cast<int32>(EPlayerStats::EPS_Count); ++i)
	{
		const EPlayerStats Stat = static_cast<EPlayerStats>(i);
		const int32* Level = Build.StatLevels.Find(Stat);

		if (!Level && !bStatSlotFree) continue; // slots full: only upgrades to owned stats
		if (Stat == EPlayerStats::EPS_Regen && Build.StatLevels.Contains(EPlayerStats::EPS_LifeSteal)) continue;
		if (Stat == EPlayerStats::EPS_LifeSteal && Build.StatLevels.Contains(EPlayerStats::EPS_Regen)) continue;

		Stats.Add(MakeStatOffer(Stat, Level ? *Level : 0));
	}

	// ---- Pick: shuffle, take 2 + 2, backfill whichever kind ran short, then heals
	Algo::RandomShuffle(Weapons);
	Algo::RandomShuffle(Stats);

	TArray<FUpgradeOffer> Result;
	TakeUpTo(Weapons, Count / 2, Result);
	TakeUpTo(Stats, Count - Result.Num(), Result);
	TakeUpTo(Weapons, Count - Result.Num(), Result);
	while (Result.Num() < Count)
	{
		Result.Add(FUpgradeOffer()); // Kind = Heal
	}

	Algo::RandomShuffle(Result); // so weapons aren't always on the left
	return Result;
}