// Survival Unleashed - Lightning Staff

#pragma once

#include "CoreMinimal.h"
#include "Weapons/StaffWeapon.h"
#include "LightningStaff.generated.h"

// No projectiles: every cooldown, instantly zaps the nearest enemies in range (one per shot) and stuns them.
UCLASS()
class SURVIVALUNLEASHED_API ALightningStaff : public AStaffWeapon
{
	GENERATED_BODY()

protected:
	virtual void Attack() override;

private:
	void ShowBeam(const FVector& Start, const FVector& End) const;
};
