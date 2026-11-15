// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GauntletData.generated.h"

struct FGauntletState;
class UBoxComponent;
/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UGauntletData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	FGauntletState*                                                                   State = nullptr;
	UPROPERTY(EditDefaultsOnly) float                                                 Direction;
	UPROPERTY(EditDefaultsOnly) TObjectPtr<USceneComponent>                           GauntletPivot;
	UPROPERTY(EditDefaultsOnly) TObjectPtr<USkeletalMeshComponent>                    Mesh;
	UPROPERTY(EditDefaultsOnly) TObjectPtr<UBoxComponent>                             BoxCollision;
};