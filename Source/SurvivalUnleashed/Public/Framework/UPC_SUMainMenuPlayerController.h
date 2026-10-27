// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UPC_SUMainMenuPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API AUPC_SUMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

	void BeginPlay() override;

public:

	UPROPERTY()
	UUserWidget* WidgetObject;

	UPROPERTY(EditDefaultsOnly, Category = "WidgetClass")
	TSubclassOf<UUserWidget> WidgetClass;

};
