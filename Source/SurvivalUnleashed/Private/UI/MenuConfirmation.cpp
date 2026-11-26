// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MenuConfirmation.h"
#include "UI/ButtonBase.h"
#include "EngineUtils.h"
#include "Framework/UGI_SUGameInstance.h"
#include "../SurvivalUnleashed.h"

UMenuConfirmation::UMenuConfirmation()
{
	ZOrderLayer = 2;
}

void UMenuConfirmation::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance()))
	{
		YesButton->CodeButtonClick.AddDynamic(GI, &UUGI_SUGameInstance::QuitToMenu);
		NoButton->CodeButtonClick.AddDynamic(this, &UMenuConfirmation::NoMenuButtonPressed);
	}

}

void UMenuConfirmation::NoMenuButtonPressed()
{
	OnMenuClosed.Broadcast();
	RemoveFromParent();
}
