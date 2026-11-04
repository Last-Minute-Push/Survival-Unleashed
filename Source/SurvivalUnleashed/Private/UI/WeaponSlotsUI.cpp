// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WeaponSlotsUI.h"

void UWeaponSlotsUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();

}

void UWeaponSlotsUI::NativeConstruct()
{
	Super::NativeConstruct();

}

void UWeaponSlotsUI::SetWeaponSlot(EWeaponCategory Category, int WeaponSlot)
{

}

void UWeaponSlotsUI::SetWeaponSlotLevel(int placeholder)
{
}

int UWeaponSlotsUI::SetWeaponImageIndex(EWeaponCategory Category)
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
