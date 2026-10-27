// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyData.generated.h"

/**
 *
 */
UCLASS()
class SURVIVALUNLEASHED_API UEnemyData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "1"))
	float MaxHealth = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "0"))
	float MoveSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "0"))
	float ContactDamage = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "0.1"))
	float ContactDamageInterval = 0.5f;

	// 0 = full knockback, 1 = immune (Stone Tortoise, bosses)
	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "0", ClampMax = "1"))
	float KnockbackResistance = 0.f;

	// Tornado pull has no effect (Stone Tortoise, elites, bosses)
	UPROPERTY(EditDefaultsOnly, Category = "Status")
	bool bIgnoresPull = false;

	// The enemy slides and slows to a stop over this time
	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "0.01"))
	float KnockbackDuration = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "Rewards", meta = (ClampMin = "0"))
	int32 ExperienceValue = 1;

	// Chance (0-1) to also drop a health pickup on death
	UPROPERTY(EditDefaultsOnly, Category = "Rewards", meta = (ClampMin = "0", ClampMax = "1"))
	float HealthDropChance = 0.01f;
};