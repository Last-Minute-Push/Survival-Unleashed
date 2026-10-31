// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OptionsMenuBase.h"
#include "UI/ButtonBase.h"
#include "Components/WidgetSwitcher.h"

UOptionsMenuBase::UOptionsMenuBase()
{
	ZOrderLayer = 2;
}

void UOptionsMenuBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BackButton->CodeButtonClick.AddDynamic(this, &UOptionsMenuBase::BackPressed);
	AudioButton->CodeButtonClick.AddDynamic(this, &UOptionsMenuBase::AudioPressed);
	GraphicsButton->CodeButtonClick.AddDynamic(this, &UOptionsMenuBase::GraphicsPressed);
	ControlsButton->CodeButtonClick.AddDynamic(this, &UOptionsMenuBase::ControlsPressed);
}

void UOptionsMenuBase::NativeConstruct()
{
	Super::NativeConstruct();
	OptionsSwitcher->SetActiveWidgetIndex(0);
}

void UOptionsMenuBase::BackPressed()
{
	//UNPARENTS WIDGET AND APPLIES OPTIONS
	//Persistance->
	//
	//

	RemoveFromParent();
	OnClosed.Broadcast();
}

void UOptionsMenuBase::AudioPressed()
{
	OptionsSwitcher->SetActiveWidgetIndex(1);
}

void UOptionsMenuBase::GraphicsPressed()
{
	OptionsSwitcher->SetActiveWidgetIndex(2);
}

void UOptionsMenuBase::ControlsPressed()
{
	OptionsSwitcher->SetActiveWidgetIndex(3);
}
