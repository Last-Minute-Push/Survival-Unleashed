// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UPC_SUMainMenuPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogMainMenuController, Log, All);

void AUPC_SUMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;

	

	if (WidgetClass) {
		WidgetObject = CreateWidget<UUserWidget>(this, WidgetClass);
		WidgetObject->AddToViewport();

		if (WidgetObject != nullptr) {
			WidgetObject->SetKeyboardFocus();
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
			InputMode.SetWidgetToFocus(WidgetObject->TakeWidget());
			this->SetInputMode(InputMode);
			this->FlushPressedKeys();
			HUDInputSetUp();
		}
	}
}

void AUPC_SUMainMenuPlayerController::HUDInputSetUp()
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(Exit, ETriggerEvent::Started, this, &AUPC_SUMainMenuPlayerController::BroadcastInputs);
	}
	else
	{
		UE_LOG(LogMainMenuController, Error, TEXT("InputComponent Cast didn't work"));
	}
}

void AUPC_SUMainMenuPlayerController::BroadcastInputs()
{
	OnExitHUD.Broadcast();
}

