// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WeaponSlotsUI.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"

void UWeaponSlotsUI::NativeConstruct()
{
	Super::NativeConstruct();
	WeaponIndexCategory.Reserve(3);
}

void UWeaponSlotsUI::SetWeaponSlot(EWeaponCategory Category, int Level)
{
	bool found = false;

	for (int i = 0; i < WeaponIndexCategory.Num(); i++)
	{
		if (WeaponIndexCategory[i] == Category)
		{
			SetWeaponSlotImageandLevel(Category, Level, i);
			found = true;
			break;
		}
	}

	if (found == false && WeaponIndexCategory.Num() != 3)
	{
		WeaponIndexCategory.Push(Category);
		SetWeaponSlotImageandLevel(Category, Level, WeaponIndexCategory.Num() - 1);
	}
	
}

void UWeaponSlotsUI::SetWeaponSlotImageandLevel(EWeaponCategory Category, int Level, int index)
{
	UWidgetSwitcher* Current = nullptr;
	UTextBlock* CurrentLevel = nullptr;

	switch(index)
	{
	case 0:
		Current = WeaponIconSwitcherOne;
		CurrentLevel = SlotOneLevel;
		break;
	case 1:
		Current = WeaponIconSwitcherTwo;
		CurrentLevel = SlotTwoLevel;
		break;
	case 2:
		Current = WeaponIconSwitcherThree;
		CurrentLevel = SlotThreeLevel;
		break;
	}

	if (Current)
	{
		Current->SetActiveWidgetIndex(GetWeaponImageIndex(Category));
		CurrentLevel->SetText(FText::Format(FText::FromString(TEXT("{0} {1}")), FText::FromString("Level : "), FText::AsNumber(Level+1)));
	}
}

int UWeaponSlotsUI::GetWeaponImageIndex(EWeaponCategory Category)
{
	
	switch (Category)
	{
	case EWeaponCategory::EBP_Sword:
		return 1;
		break;
	case EWeaponCategory::EBP_Gauntlets:
		return 2;
		break;
	case EWeaponCategory::EBP_Axe:
		return 3;
		break;
	case EWeaponCategory::EBP_Spear:
		return 4;
		break;
	case EWeaponCategory::EBP_BowToGun:
		return 5;
	case EWeaponCategory::EBP_IceStaff:
		return 6;
		break;
	case EWeaponCategory::EBP_EarthStaff:
		return 7;
		break;
	case EWeaponCategory::EBP_FireStaff:
		return 8;
		break;
	case EWeaponCategory::EBP_LightningStaff:
		return 9;
		break;
	}
	return 0;
}
