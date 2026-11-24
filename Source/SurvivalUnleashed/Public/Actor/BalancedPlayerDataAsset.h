// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimInstance.h"
#include "Actor/Animation/PlayerAnimInstance.h"
#include "Engine/SkeletalMesh.h"
#include "Weapons/WeaponDatabase.h"
#include "BalancedPlayerDataAsset.generated.h"


UENUM(BlueprintType)
enum class EPassives : uint8
{
	Villager,
	Wizard,
	Fighter,
	Ranger
};


UCLASS()
class SURVIVALUNLEASHED_API UBalancedPlayerDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float MaxHealth = 100;

	UPROPERTY()
	float CurrentHealth = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float DashCooldown = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float DashLength = 0.25;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float Regen = 0; // this is in seconds

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float ProjectileAmount = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float PickupRange = 160;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float MoveSpeed = 800;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float LifeSteal = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float Evasion = 0;


	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float Attack = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float WeaponAttack = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float Defense = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float AttackSpeed = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float ProjectileSpeed = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float HealAmount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float EXPGain = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float Reflect = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Cost")
	float UnlockCost = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Model")
	USkeletalMesh* Model;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> Animation;

	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TMap<EWeaponCategory, int32> StartingWeapons;

	UPROPERTY(EditDefaultsOnly, Category = "Passive")
	EPassives Passive;

	UPROPERTY(EditDefaultsOnly, Category = "Unlock")
	bool Unlocked = false;
};
