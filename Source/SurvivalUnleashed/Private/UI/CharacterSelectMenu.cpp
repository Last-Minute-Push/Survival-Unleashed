// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CharacterSelectMenu.h"
#include "Components/TextBlock.h"
#include "Actor/BalancedPlayerDataAsset.h"
#include "UI/ButtonBase.h"


void UCharacterSelectMenu::NativePreConstruct()
{
	GI = Cast<UUGI_SUGameInstance>(GetGameInstance());
}

void UCharacterSelectMenu::NativeOnInitialized()
{
	if (CharacterOption_1 && CharacterOption_2 && CharacterOption_3 && CharacterOption_4) {
		CharacterOption_1->CodeButtonClick.AddDynamic(this, &UCharacterSelectMenu::ChooseCharacterV);
		CharacterOption_2->CodeButtonClick.AddDynamic(this, &UCharacterSelectMenu::ChooseCharacterF);
		CharacterOption_3->CodeButtonClick.AddDynamic(this, &UCharacterSelectMenu::ChooseCharacterR);
		CharacterOption_4->CodeButtonClick.AddDynamic(this, &UCharacterSelectMenu::ChooseCharacterW);
		BackUpgradeButton->CodeButtonClick.AddDynamic(this, &UCharacterSelectMenu::BackButton);
		CharacterLock();
	}
	SetPointsText(0);
}

void UCharacterSelectMenu::CharacterLock()
{
	if (CharacterOption_2 && CharacterOption_3 && CharacterOption_4) {
		CharacterOption_2->LockCheck();
		CharacterOption_3->LockCheck();
		CharacterOption_4->LockCheck();
	}
}

void UCharacterSelectMenu::ChooseCharacterV()
{

	if (GI) {
		GI->PlayerData = GI->VillagerData;
	}
}

void UCharacterSelectMenu::ChooseCharacterF()
{
	if (CharacterOption_2->Locked) {
		CharacterUnlock(CharacterOption_2);
	}
	else {
		if (GI) {
			GI->PlayerData = GI->FighterData;
		}
	}

}

void UCharacterSelectMenu::ChooseCharacterR()
{
	if (CharacterOption_2->Locked) {
		CharacterUnlock(CharacterOption_2);
	}
	else {
		if (GI) {
			GI->PlayerData = GI->RangerData;
		}
	}

}

void UCharacterSelectMenu::ChooseCharacterW()
{
	if (CharacterOption_4->Locked) {
		CharacterUnlock(CharacterOption_4);
	}
	else {
		if (GI) {
			GI->PlayerData = GI->WizardData;
		}
	}

}

void UCharacterSelectMenu::BackButton()
{
	RemoveFromParent();
	OnBack.Broadcast();
}

void UCharacterSelectMenu::CharacterUnlock(UCharacterButton* Character)
{

	if (Character == CharacterOption_2) {
		UBalancedPlayerDataAsset* Data = Cast<UBalancedPlayerDataAsset>(GI->FighterData);
		if (CurrentPointsAmount >= Data->UnlockCost) {
			CharacterOption_2->Locked = false;
			CharacterOption_2->LockCheck();
			CurrentPointsAmount -= Data->UnlockCost;
			SetPointsText(CurrentPointsAmount);
			if (GI) {
				GI->PlayerData = GI->FighterData;
			}
		}
		else {
			//YOU'RE BROKE!!!!!!!!!!!!
		}

	}
	else if (Character == CharacterOption_3) {
		UBalancedPlayerDataAsset* Data = Cast<UBalancedPlayerDataAsset>(GI->RangerData);
		if (CurrentPointsAmount >= Data->UnlockCost) {
			CharacterOption_3->Locked = false;
			CharacterOption_3->LockCheck();
			CurrentPointsAmount -= Data->UnlockCost;
			SetPointsText(CurrentPointsAmount);
			if (GI) {
				GI->PlayerData = GI->RangerData;
			}
		}
		else {
			//YOU'RE BROKE!!!!!!!!!!!!
		}
	}
	else if (Character == CharacterOption_4) {
		UBalancedPlayerDataAsset* Data = Cast<UBalancedPlayerDataAsset>(GI->WizardData);
		if (CurrentPointsAmount >= Data->UnlockCost) {
			CharacterOption_4->Locked = false;
			CharacterOption_4->LockCheck();
			CurrentPointsAmount -= Data->UnlockCost;
			SetPointsText(CurrentPointsAmount);
			if (GI) {
				GI->PlayerData = GI->WizardData;
			}
		}
		else {
			//YOU'RE BROKE!!!!!!!!!!!!
		}
	}

}

void UCharacterSelectMenu::SetPointsAmount(float CurrentPoints)
{
	CurrentPointsAmount = CurrentPoints;
}

void UCharacterSelectMenu::SetPointsText(int Ppoints)
{
	FText text;
	text = FText::FromString("Perma Points: " + FString::SanitizeFloat(Ppoints));
	Points->SetText(text);
}
