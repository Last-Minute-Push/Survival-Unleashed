// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/SplashScreenPlayerController.h"

void ASplashScreenPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (WidgetClass) {
		WidgetObject = CreateWidget<UUserWidget>(this, WidgetClass);
		WidgetObject->AddToViewport();

		if (WidgetObject != nullptr) {
			WidgetObject->SetKeyboardFocus();
			
			FInputModeUIOnly InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
			InputMode.SetWidgetToFocus(WidgetObject->TakeWidget());
			this->SetInputMode(InputMode);
			this->FlushPressedKeys();
		}
	}
}
