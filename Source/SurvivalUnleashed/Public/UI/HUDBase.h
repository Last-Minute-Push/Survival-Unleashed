// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDBase.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UHUDBase : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Layering", meta = (ClampMin = "0")) int32 ZOrderLayer = 0;

public:
	void AddToViewport();
	void AddToViewport(int32 ZOrder);
};
