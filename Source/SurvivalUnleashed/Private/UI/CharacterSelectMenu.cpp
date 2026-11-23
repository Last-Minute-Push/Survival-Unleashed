// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CharacterSelectMenu.h"
#include "Components/TextBlock.h"
#include "Actor/BalancedPlayerDataAsset.h"
#include "Components/Image.h"


void UCharacterSelectMenu::NativePreConstruct()
{
	GI = Cast<UUGI_SUGameInstance>(GetGameInstance());
}

void UCharacterSelectMenu::NativeOnInitialized()
{
	GI = Cast<UUGI_SUGameInstance>(GetGameInstance());
	if (CharacterOption_1 && CharacterOption_2 && CharacterOption_3 && CharacterOption_4) {
		CharacterOption_1->CodeButtonClick.AddDynamic(this, &UCharacterSelectMenu::ChooseCharacterV);
		CharacterOption_2->CodeButtonClick.AddDynamic(this, &UCharacterSelectMenu::ChooseCharacterF);
		CharacterOption_3->CodeButtonClick.AddDynamic(this, &UCharacterSelectMenu::ChooseCharacterR);
		CharacterOption_4->CodeButtonClick.AddDynamic(this, &UCharacterSelectMenu::ChooseCharacterW);
		if (BackUpgradeButton && PlayButton && GI) {
			PlayButton->CodeButtonClick.AddDynamic(GI, &UUGI_SUGameInstance::LoadFirstLevel);
			BackUpgradeButton->CodeButtonClick.AddDynamic(this, &UCharacterSelectMenu::BackButton);
		}
		if (CharacterImage2&& CharacterImage3 && CharacterImage4 ){
			CharacterOption_2->CharacterImage->SetBrushFromTexture(CharacterImage2);
			CharacterOption_3->CharacterImage->SetBrushFromTexture(CharacterImage3);
			CharacterOption_4->CharacterImage->SetBrushFromTexture(CharacterImage4);
			if (GI) {
				UBalancedPlayerDataAsset* Data2 = Cast<UBalancedPlayerDataAsset>(GI->FighterData);
				UBalancedPlayerDataAsset* Data3 = Cast<UBalancedPlayerDataAsset>(GI->RangerData);
				UBalancedPlayerDataAsset* Data4 = Cast<UBalancedPlayerDataAsset>(GI->WizardData);
				CharacterOption_2->SetPriceText(FText::FromString("Cost: " + FString::FromInt((int)Data2->UnlockCost)));
				CharacterOption_3->SetPriceText(FText::FromString("Cost: " + FString::FromInt((int)Data3->UnlockCost)));
				CharacterOption_4->SetPriceText(FText::FromString("Cost: " + FString::FromInt((int)Data4->UnlockCost)));
			}
		}
		CharacterLock();
	}
	if (Points) {
		SetPointsText(0);
	}

}

void UCharacterSelectMenu::CharacterLock()
{
	if (CharacterOption_2 && CharacterOption_3 && CharacterOption_4) {
		UBalancedPlayerDataAsset* Data2 = Cast<UBalancedPlayerDataAsset>(GI->FighterData);
		UBalancedPlayerDataAsset* Data3 = Cast<UBalancedPlayerDataAsset>(GI->RangerData);
		UBalancedPlayerDataAsset* Data4 = Cast<UBalancedPlayerDataAsset>(GI->WizardData);
		if (Data2->Unlocked) {
			CharacterOption_2->Locked = false;
		}
		CharacterOption_2->LockCheck();
		if (Data3->Unlocked) {
			CharacterOption_3->Locked = false;
		}
		CharacterOption_3->LockCheck();
		if (Data4->Unlocked) {
			CharacterOption_4->Locked = false;
		}
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
	if (CharacterOption_3->Locked) {
		CharacterUnlock(CharacterOption_3);
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
			Data->Unlocked = true;
			CharacterLock();
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
			Data->Unlocked = true;
			CharacterLock();
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
			Data->Unlocked = true;
			CharacterLock();
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
	if (Points) {
		Points->SetText(text);
	}
}