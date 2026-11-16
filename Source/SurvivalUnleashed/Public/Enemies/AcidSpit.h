// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/ProjectileBase.h"
#include "AcidSpit.generated.h"

/**
 * The Pitcher Plant's acid glob. A pooled projectile like the player's, turned around:
 * it hits the player and passes through enemies. Obstacles stop it (ProjectileBase handles that).
 * Damage is applied by whoever fired it, through the hit callback.
 */
UCLASS()
class SURVIVALUNLEASHED_API AAcidSpit : public AProjectileBase
{
	GENERATED_BODY()

public:
	AAcidSpit();

protected:
	virtual bool IsValidTarget(const AActor* Target) const override;
};
