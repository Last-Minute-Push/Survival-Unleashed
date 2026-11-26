// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/QuitConfirmation.h"
#include "UI/ButtonBase.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Framework/UGI_SUGameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuitConfirm, Log, All);

UQuitConfirmation::UQuitConfirmation()
{
	ZOrderLayer = 0;
	bIsFocusable = true;
}

void UQuitConfirmation::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	

	if (UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance()))
	{
		YesButton->CodeButtonClick.AddDynamic(GI, &UUGI_SUGameInstance::QuitTheGame);
		NoButton->CodeButtonClick.AddDynamic(this, &UQuitConfirmation::NoButtonPressed);
	}

}

FReply UQuitConfirmation::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		NoButtonPressed();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UQuitConfirmation::NoButtonPressed()
{
	OnQuitClosed.Broadcast();
	RemoveFromParent();
}

