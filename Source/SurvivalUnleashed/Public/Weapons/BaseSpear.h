// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/BaseBowToGun.h"
#include "BaseSpear.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API ABaseSpear : public ABaseBowToGun
{
	GENERATED_BODY()

	float Range;     // Data->Range	
	float Height;    // Data->Height
	float Radius;    // Data->Radius
	float Damage;    // Data->Damage
	float Cooldown;  // Data->Cooldown * 0.5f (one half for TFireCooldown, the other half for TAimDelay)
	float Knockback; // Data->Knockback
	float Speed;     // Data->AttackSpeed
	
	void StartThrust();
	void StopThrust();

protected:
	virtual void BeginPlay();
};
