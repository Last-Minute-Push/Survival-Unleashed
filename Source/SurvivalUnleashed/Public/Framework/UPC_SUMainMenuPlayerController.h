// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "InputAction.h"
#include "UPC_SUMainMenuPlayerController.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExitHUD);
UCLASS()
class SURVIVALUNLEASHED_API AUPC_SUMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

	void BeginPlay() override;

	void HUDInputSetUp();

	void BroadcastInputs();

public:

	UPROPERTY()
	UUserWidget* WidgetObject;

	UPROPERTY(EditDefaultsOnly, Category = "WidgetClass")
	TSubclassOf<UUserWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* Exit;

	FOnExitHUD OnExitHUD;

};
