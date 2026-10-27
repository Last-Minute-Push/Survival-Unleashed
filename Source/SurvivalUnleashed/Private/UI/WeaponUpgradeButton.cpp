// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WeaponUpgradeButton.h"

void UWeaponUpgradeButton::NativeOnInitialized()
{
	Super::Super::NativeOnInitialized();

	if (TheButton)
	{
		TheButton->OnClicked.AddDynamic(this, &UWeaponUpgradeButton::HandleUpgradeBackgroundClick);
		TheButton->OnHovered.AddDynamic(this, &UButtonBase::HandleHoverSFX);
	}
}

void UWeaponUpgradeButton::HandleUpgradeBackgroundClick()
{
	UpgradeButtonClick.Broadcast(Type);
}