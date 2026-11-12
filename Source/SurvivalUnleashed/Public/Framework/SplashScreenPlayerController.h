// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "SplashScreenPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API ASplashScreenPlayerController : public APlayerController
{
	GENERATED_BODY()

	void BeginPlay() override;
	

public:

	UPROPERTY()
	UUserWidget* WidgetObject;

	UPROPERTY(EditDefaultsOnly, Category = "WidgetClass")
	TSubclassOf<UUserWidget> WidgetClass;
	
};
