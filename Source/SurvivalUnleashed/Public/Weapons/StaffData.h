// Survival Unleashed - data for the elemental staves (SCRUM: Add elemental staffs)

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponData.h"
#include "Enemies/EnemyBase.h" // EEnemyStatus
#include "StaffData.generated.h"

class UNiagaraSystem;

// One asset per element (DA_FireStaff, DA_IceStaff, ...). Every level of a staff uses the same asset;
// the weapon's upgrade index (0 = level 1) picks the row from the per-level arrays.
// Inherited fields used: Cooldown, Range, Knockback, Alignment, ProjectileType.
UCLASS()
class SURVIVALUNLEASHED_API UStaffData : public UWeaponData
{
	GENERATED_BODY()

public:
	// Damage per hit before player stats. Index 0 = level 1; past the end, the last entry is used.
	UPROPERTY(EditDefaultsOnly, Category = "Staff|Levels")
	TArray<float> DamagePerLevel = { 6.f, 8.f, 10.f, 13.f, 16.f, 20.f };

	// Projectiles (or zaps) per attack, before the player's Projectile Amount stat
	UPROPERTY(EditDefaultsOnly, Category = "Staff|Levels")
	TArray<int32> ProjectilesPerLevel = { 1, 1, 2, 3, 3, 4 };

	UPROPERTY(EditDefaultsOnly, Category = "Staff|Projectile", meta = (ClampMin = "0"))
	float ProjectileSpeed = 1200.f;

	// Extra enemies a projectile passes through: 0 = stops at the first, -1 = unlimited
	UPROPERTY(EditDefaultsOnly, Category = "Staff|Projectile", meta = (ClampMin = "-1"))
	int32 Pierce = 0;

	// Above 0: the first hit explodes and hits everything in this radius (Fire Staff)
	UPROPERTY(EditDefaultsOnly, Category = "Staff|Effect", meta = (ClampMin = "0"))
	float ExplosionRadius = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Staff|Effect")
	bool bAppliesStatus = false;

	UPROPERTY(EditDefaultsOnly, Category = "Staff|Effect", meta = (EditCondition = "bAppliesStatus"))
	EEnemyStatus Status = EEnemyStatus::Burn;

	UPROPERTY(EditDefaultsOnly, Category = "Staff|Effect", meta = (EditCondition = "bAppliesStatus", ClampMin = "0"))
	float StatusDuration = 3.f;

	// Damage every 0.5 s for Burn and Freeze; unused for Stun
	UPROPERTY(EditDefaultsOnly, Category = "Staff|Effect", meta = (EditCondition = "bAppliesStatus", ClampMin = "0"))
	float StatusMagnitude = 2.f;

	// Optional. Projectile staves: spawned where a projectile hits (the Fire Staff's explosion).
	// Lightning: a beam system with a Vector user parameter named "BeamEnd".
	UPROPERTY(EditDefaultsOnly, Category = "Staff|Effect")
	TObjectPtr<UNiagaraSystem> HitEffect;

	float GetLevelDamage(int32 Upgrade) const
	{
		return DamagePerLevel.IsEmpty() ? Damage : DamagePerLevel[FMath::Clamp(Upgrade, 0, DamagePerLevel.Num() - 1)];
	}

	int32 GetLevelProjectiles(int32 Upgrade) const
	{
		return ProjectilesPerLevel.IsEmpty() ? 1 : ProjectilesPerLevel[FMath::Clamp(Upgrade, 0, ProjectilesPerLevel.Num() - 1)];
	}
};
