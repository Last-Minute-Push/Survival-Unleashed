// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UpgradeButton.h"
#include "UI/ButtonBase.h"
//#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "../SurvivalUnleashed.h"

DEFINE_LOG_CATEGORY_STATIC(LogUpgradeButton, Log, All);

UUpgradeButton::UUpgradeButton()
{
	ZOrderLayer = 2;
}

void UUpgradeButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	TextButton->SetButtonText(UpgradeText);
	TextButton->SetButtonFont(UpgradeFontInfo);
	TextButton->SetButtonClickSFX(UpgradeSound);
}

void UUpgradeButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (TextButton->TheButton)
	{
		TextButton->CodeButtonClick.AddDynamic(this, &UUpgradeButton::HandleUpgradeButtonClick);
	}
}



void UUpgradeButton::SetPipRank(int Rank)
{
	
	switch (Rank)
	{
		case 1:
			SetPipColors(0);
		break;
		
		case 2:
			SetPipColors(1);
			break;

		case 3:
			SetPipColors(2);
			break;

		case 4:
			SetPipColors(3);
			break;

		case 5:
			SetPipColors(4);
			break;

	}
}

void UUpgradeButton::SetPipColors(int Index)
{
	UImage* IndexPip = Cast<UImage>(PipBoxHolder->GetChildAt(Index));
	
	if (IndexPip)
	{
		IndexPip->SetBrushTintColor(PipUpgradeColor);
	}
	else
	{
		UE_LOG(LogUpgradeButton, Error, TEXT("Pip index not available, check back in UpgradeButtonFile!"));
	}
}

void UUpgradeButton::HandleUpgradeButtonClick()
{

	if (CurrentRank < 5)
	{
		CurrentRank++;

		SetPipRank(CurrentRank);

	}
}
