// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/PermanentUpgradeRules.h"

UPermanentUpgradeRules::UPermanentUpgradeRules()
{
	BonusPerRank = {
		{ EPlayerStats::EPS_Health,           5.f    }, // +5 max HP (5% of the base 100), before focus
		{ EPlayerStats::EPS_Defense,          3.f    }, // +3 Defense
		{ EPlayerStats::EPS_Attack,           0.025f }, // +2.5% damage (half of a level-up's +5%)
		{ EPlayerStats::EPS_WeaponAttack,     0.05f  }, // +5% weapon damage
		{ EPlayerStats::EPS_AttackSpeed,      0.04f  }, // +4% attack speed
		{ EPlayerStats::EPS_DashCooldown,     0.04f  }, // -4% dash cooldown
		{ EPlayerStats::EPS_DashLength,       0.05f  }, // +5% dash length
		{ EPlayerStats::EPS_Reflect,          0.05f  }, // +5% damage reflected
		{ EPlayerStats::EPS_Regen,            0.5f   }, // +0.5 HP per regen tick (read by the healing ticket, SCRUM-650)
		{ EPlayerStats::EPS_LifeSteal,        0.02f  }, // +2% life steal chance
		{ EPlayerStats::EPS_Evasion,          0.02f  }, // +2% evasion chance
		{ EPlayerStats::EPS_ProjectileAmount, 1.f    }, // +1 projectile
		{ EPlayerStats::EPS_ProjectileSpeed,  0.05f  }, // +5% projectile speed
		{ EPlayerStats::EPS_HealAmount,       0.05f  }, // +5% healing
		{ EPlayerStats::EPS_EXPGain,          0.04f  }, // +4% experience
		{ EPlayerStats::EPS_PickupRange,      30.f   }, // +30 uu pickup range
	};
}

int32 UPermanentUpgradeRules::GetNextRankCost(int32 CurrentRank) const
{
	return RankCosts.IsValidIndex(CurrentRank) ? RankCosts[CurrentRank] : -1;
}

FName UPermanentUpgradeRules::GetStatKey(EPlayerStats Stat)
{
	return FName(*StaticEnum<EPlayerStats>()->GetNameStringByValue(static_cast<int64>(Stat)));
}
