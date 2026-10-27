// Survival Unleashed - Fire, Ice, and Earth staves (SCRUM: Add elemental staffs)

#include "Weapons/StaffWeapon.h"
#include "Weapons/StaffData.h"
#include "Weapons/ProjectileBase.h"
#include "Enemies/EnemyBase.h"
#include "Enemies/HordeSubsystem.h"
#include "Actor/UC_Player.h"
#include "Utility/PlayerStatsComponent.h"
#include "Framework/ObjectPoolSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogStaff, Log, All);

void AStaffWeapon::BeginPlay()
{
	Super::BeginPlay();

	const UStaffData* SD = GetStaffData();
	if (!SD)
	{
		UE_LOG(LogStaff, Error, TEXT("%s needs a StaffData asset in its Data slot"), *GetName());
		return;
	}

	// Upgrading destroys and respawns the staff, so only prewarm the first time this projectile type shows up.
	// Otherwise every level-up would add another full set to the pool.
	if (SD->ProjectileType)
	{
		UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
		if (Pool && Pool->GetInactiveCount(SD->ProjectileType) == 0)
		{
			AProjectileBase::Prewarm(GetWorld(), SD->ProjectileType);
		}
	}

	// The weapon subsystem sets Parent and the upgrade level right after BeginPlay, so the first attack
	// waits one cooldown and everything reads them at attack time instead of here.
	ScheduleNextAttack();
}

void AStaffWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
	Super::EndPlay(EndPlayReason);
}

void AStaffWeapon::StopWeapon()
{
	Super::StopWeapon();
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

const UStaffData* AStaffWeapon::GetStaffData() const
{
	return Cast<UStaffData>(Data);
}

float AStaffWeapon::GetHitDamage() const
{
	const UStaffData* SD = GetStaffData();
	return SD ? ComputeHitFromBase(SD->GetLevelDamage(GetWeaponUpgrade()), ParentStats) : Super::GetHitDamage();
}

FVector AStaffWeapon::GetFireOrigin() const
{
	return Parent ? Parent->GetActorLocation() : GetActorLocation();
}

int32 AStaffWeapon::GetShotCount() const
{
	const UStaffData* SD = GetStaffData();
	int32 Shots = SD ? SD->GetLevelProjectiles(GetWeaponUpgrade()) : 1;
	if (ParentStats)
	{
		// The stat starts at 1 (one projectile), so only what's above 1 is extra
		Shots += FMath::Max(0, FMath::FloorToInt(ParentStats->GetProjectileAmount()) - 1);
	}
	return FMath::Max(Shots, 1);
}

void AStaffWeapon::FindTargets(int32 Count, float Range, TArray<AEnemyBase*>& OutTargets) const
{
	OutTargets.Reset();
	if (UHordeSubsystem* Horde = GetWorld()->GetSubsystem<UHordeSubsystem>())
	{
		Horde->GetNearestEnemies(GetFireOrigin(), Range, Count, OutTargets);
	}
}

void AStaffWeapon::ScheduleNextAttack()
{
	const UStaffData* SD = GetStaffData();
	float Cooldown = SD ? SD->Cooldown : 1.f;
	if (ParentStats && ParentStats->GetAttackSpeed() > 0.f)
	{
		Cooldown /= ParentStats->GetAttackSpeed(); // Attack Speed is a multiplier: 1.0 = base, 1.05 = 5% faster
	}
	// Recomputed every attack, so Attack Speed upgrades apply right away
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AStaffWeapon::OnAttackTimer, FMath::Max(Cooldown, 0.1f), false);
}

void AStaffWeapon::OnAttackTimer()
{
	Attack();
	ScheduleNextAttack();
}

void AStaffWeapon::Attack()
{
	const UStaffData* SD = GetStaffData();
	if (!SD || !SD->ProjectileType || !Parent)
	{
		return;
	}

	const int32 Shots = GetShotCount();
	TArray<AEnemyBase*> Targets;
	FindTargets(Shots, SD->Range, Targets);
	if (Targets.IsEmpty())
	{
		return; // nothing in range; try again next cooldown
	}

	FProjectileLaunchParams Params;
	Params.Speed = SD->ProjectileSpeed;
	Params.Range = SD->Range;
	Params.Pierce = SD->Pierce;

	const FVector Origin = GetFireOrigin();
	for (int32 i = 0; i < Shots; ++i)
	{
		// One shot per target, nearest first. Extra shots fan out around the targets, alternating sides.
		AEnemyBase* Target = Targets[i % Targets.Num()];
		const FVector TargetLocation = Target->GetActorLocation();

		FVector Direction = (TargetLocation - Origin).GetSafeNormal2D();
		if (i >= Targets.Num())
		{
			const int32 Extra = i - Targets.Num();
			const float Angle = 12.f * (Extra / 2 + 1) * ((Extra % 2) ? 1.f : -1.f);
			Direction = Direction.RotateAngleAxis(Angle, FVector::UpVector);
		}

		// Fly level at the target's height so short enemies don't get shot over
		Params.Origin = FVector(Origin.X, Origin.Y, TargetLocation.Z);
		Params.Direction = Direction;
		AProjectileBase::Fire(GetWorld(), SD->ProjectileType, this, Params,
			FOnProjectileHit::CreateUObject(this, &AStaffWeapon::OnProjectileHit));
	}
}

void AStaffWeapon::OnProjectileHit(AProjectileBase* Projectile, AActor* Target)
{
	AEnemyBase* Enemy = Cast<AEnemyBase>(Target);
	const UStaffData* SD = GetStaffData();
	if (!Enemy || !SD)
	{
		return;
	}

	const FVector Center = Enemy->GetActorLocation();
	SpawnHitEffect(Center);

	if (SD->ExplosionRadius <= 0.f)
	{
		HitEnemy(Projectile, Enemy);
		return;
	}

	// Explosion: everything in the radius, including the enemy that was hit
	TArray<AEnemyBase*> Caught;
	if (UHordeSubsystem* Horde = GetWorld()->GetSubsystem<UHordeSubsystem>())
	{
		Horde->GetEnemiesInRadius(Center, SD->ExplosionRadius, Caught);
	}
	Caught.AddUnique(Enemy);
	for (AEnemyBase* E : Caught)
	{
		HitEnemy(Projectile, E);
	}

#if ENABLE_DRAW_DEBUG
	if (!SD->HitEffect) // stand-in until the explosion VFX exists
	{
		DrawDebugSphere(GetWorld(), Center, SD->ExplosionRadius, 12, FColor::Orange, false, 0.15f);
	}
#endif
}

void AStaffWeapon::HitEnemy(AActor* Source, AEnemyBase* Enemy)
{
	if (!IsValid(Enemy) || Enemy->IsDead())
	{
		return;
	}

	ApplyDamage(Source, Enemy); // damage, life steal, knockback (Data->Knockback), OnWeaponHit

	// The hit may have killed it and sent it back to the pool
	const UStaffData* SD = GetStaffData();
	if (SD && SD->bAppliesStatus && !Enemy->IsDead())
	{
		Enemy->ApplyStatus(SD->Status, SD->StatusDuration, SD->StatusMagnitude, this);
	}
}

void AStaffWeapon::SpawnHitEffect(const FVector& Location) const
{
	const UStaffData* SD = GetStaffData();
	if (SD && SD->HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SD->HitEffect, Location);
	}
}