// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/QuitConfirmation.h"
#include "UI/ButtonBase.h"
#include "Framework/UGI_SUGameInstance.h"


void UQuitConfirmation::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance()))
	{
		YesButton->CodeButtonClick.AddDynamic(GI, &UUGI_SUGameInstance::QuitTheGame);
		NoButton->CodeButtonClick.AddDynamic(this, &UQuitConfirmation::NoButtonPressed);
	}

}

void UQuitConfirmation::NoButtonPressed()
{
	OnQuitClosed.Broadcast();
	RemoveFromParent();
}

