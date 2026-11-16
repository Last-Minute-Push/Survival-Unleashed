// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Credits.h"
#include "UI/ButtonBase.h"

void UCredits::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BackButton->CodeButtonClick.AddDynamic(this, &UCredits::BackButtonPressed);
		
}

void UCredits::BackButtonPressed()
{
	RemoveFromParent();
	OnCreditsClose.Broadcast();
}
