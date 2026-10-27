// Survival Unleashed - Lightning Staff

#include "Weapons/LightningStaff.h"
#include "Weapons/StaffData.h"
#include "Enemies/EnemyBase.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "DrawDebugHelpers.h"

void ALightningStaff::Attack()
{
	const UStaffData* SD = GetStaffData();
	if (!SD || !Parent)
	{
		return;
	}

	TArray<AEnemyBase*> Targets;
	FindTargets(GetShotCount(), SD->Range, Targets); // nearest first; extra zaps go to the next nearest

	const FVector Origin = GetFireOrigin();
	for (AEnemyBase* Enemy : Targets)
	{
		ShowBeam(Origin, Enemy->GetActorLocation());
		HitEnemy(this, Enemy); // damage + Stun from the data asset
	}
}

void ALightningStaff::ShowBeam(const FVector& Start, const FVector& End) const
{
	const UStaffData* SD = GetStaffData();
	if (SD && SD->HitEffect)
	{
		if (UNiagaraComponent* Beam = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SD->HitEffect, Start))
		{
			Beam->SetVariableVec3(TEXT("BeamEnd"), End);
		}
		return;
	}

#if ENABLE_DRAW_DEBUG
	DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 0.1f, 0, 4.f); // stand-in until the beam VFX exists
#endif
}