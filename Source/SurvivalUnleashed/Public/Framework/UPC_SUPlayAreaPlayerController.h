// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UPC_SUPlayAreaPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API AUPC_SUPlayAreaPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	AUPC_SUPlayAreaPlayerController();
	virtual void BeginPlay() override;
};
