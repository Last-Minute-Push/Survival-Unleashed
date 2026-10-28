// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponData.generated.h"

class UNiagaraSystem;
class AProjectileBase;

UENUM(BlueprintType)
enum class Focus : uint8
{
	Barbaric     UMETA(DisplayName = "Barbaric"),
	Intelligence UMETA(DisplayName = "Intelligence"),
	Modern       UMETA(DisplayName = "Modern"),
};

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "0"))                     float Damage;
	UPROPERTY(EditDefaultsOnly, Category = "Stats", Meta = (ClampMin = "0.1"))                   float Cooldown;
	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "0.1"))                   float Range;
	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "0"))                     float Knockback;
	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (ClampMin = "0"))                     float AttackSpeed;
	UPROPERTY(EditDefaultsOnly, Transient, Category = "Stats")                                   Focus Alignment;

	UPROPERTY(EditDefaultsOnly, Category = "Properties|Mesh", meta = (ClampMin = "0"))           float Height;
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Mesh", meta = (ClampMin = "0"))           float DistanceFromPlayer;
	UPROPERTY(EditDefaultsOnly, Category = "Properties|ProjectileMesh")                          TSubclassOf<AProjectileBase> ProjectileType;
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Projectile", meta = (ClampMin = "0"))     int32 ProjectilePierce;

	UPROPERTY(EditDefaultsOnly, Category = "Properties|Niagara")                                 UNiagaraSystem* NiagaraSystem;
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Niagara|Trails", meta = (ClampMin = "0")) float TrailWidth;
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Niagara|Trails")                          FColor TrailColor;
	UPROPERTY(EditDefaultsOnly, Category = "Properties|Niagara|Trails", meta = (ClampMin = "0")) float TrailLifespan;
};