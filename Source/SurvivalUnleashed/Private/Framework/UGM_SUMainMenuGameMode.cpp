// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UGM_SUMainMenuGameMode.h"
#include "Framework/UPC_SUMainMenuPlayerController.h"

AUGM_SUMainMenuGameMode::AUGM_SUMainMenuGameMode()
{
	PlayerControllerClass = AUPC_SUMainMenuPlayerController::StaticClass();
}
