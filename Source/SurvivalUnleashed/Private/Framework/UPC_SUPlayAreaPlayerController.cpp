// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UPC_SUPlayAreaPlayerController.h"
#include "Cheats/SUCheatManager.h"

AUPC_SUPlayAreaPlayerController::AUPC_SUPlayAreaPlayerController()
{
	CheatClass = USUCheatManager::StaticClass();
}

void AUPC_SUPlayAreaPlayerController::BeginPlay()
{

	/*
	FInputModeGameAndUI with the cursor visible isn’t a good fit for gameplay here.
	The player moves with WASD and weapons fire automatically, so nothing in gameplay uses mouse clicks. 
	With GameAndUI, a stray click on a HUD widget can grab keyboard focus, and WASD stops working until you click back into the game.
	Players will read that as “the controls broke.” So, should use GameOnly with hidden cursor instead.

	Screens that do need the mouse should switch the input mode themselves when they open, and switch back when they close

	this->bShowMouseCursor = true;
	FInputModeGameAndUI InputModeData;
	InputModeData.SetHideCursorDuringCapture(false);
	this->SetInputMode(InputModeData);
	*/

	Super::BeginPlay();
	SetInputMode(FInputModeGameOnly());
	this->bShowMouseCursor = false;
}
