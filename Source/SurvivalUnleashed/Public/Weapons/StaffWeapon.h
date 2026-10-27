// Survival Unleashed - Fire, Ice, and Earth staves

#pragma once

#include "CoreMinimal.h"
#include "Weapons/BaseWeapon.h"
#include "StaffWeapon.generated.h"

class AEnemyBase;
class AProjectileBase;
class UStaffData;

// Every cooldown, fires pooled projectiles at the nearest enemies. What a hit does (explode, status,
// knockback, pierce) comes from the UStaffData asset, so Fire, Ice, and Earth are the same class with
// different data. The Lightning Staff subclasses this and replaces the projectiles with instant zaps.
UCLASS()
class SURVIVALUNLEASHED_API AStaffWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void StopWeapon() override;

protected:
	// This level's damage through the shared weapon damage formula
	virtual float GetHitDamage() const override;

	// One attack. The Lightning Staff overrides this.
	virtual void Attack();

	// Damage, knockback, and this staff's status on one enemy
	void HitEnemy(AActor* Source, AEnemyBase* Enemy);

	// Projectiles or zaps this attack: the level's count plus the Projectile Amount stat
	int32 GetShotCount() const;

	// Up to Count nearest living enemies within Range of the player, nearest first
	void FindTargets(int32 Count, float Range, TArray<AEnemyBase*>& OutTargets) const;

	FVector GetFireOrigin() const;
	const UStaffData* GetStaffData() const;
	void SpawnHitEffect(const FVector& Location) const;

private:
	void OnAttackTimer();
	void ScheduleNextAttack();
	void OnProjectileHit(AProjectileBase* Projectile, AActor* Target);

	FTimerHandle AttackTimer;
};
