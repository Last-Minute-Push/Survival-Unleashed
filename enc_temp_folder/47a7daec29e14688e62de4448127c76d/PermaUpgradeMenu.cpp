// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PermaUpgradeMenu.h"
#include "UI/ButtonBase.h"

void UPermaUpgradeMenu::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BackUpgradeButton->CodeButtonClick.AddDynamic(this, &UPermaUpgradeMenu::BackPressed);
}

void UPermaUpgradeMenu::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPermaUpgradeMenu::BackPressed()
{
	
	RemoveFromParent();
	OnUpgradeClose.Broadcast();
}