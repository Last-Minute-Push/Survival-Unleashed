// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CharacterButton.h"
#include "Audio/Interfaces/AudioManagerInterface.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

void UCharacterButton::LockCheck()
{
	if (Locked) {
		Switcher->SetActiveWidgetIndex(1);
	}
	else {
		Switcher->SetActiveWidgetIndex(0);
	}
}

void UCharacterButton::NativePreConstruct() {
	Super::NativePreConstruct();

	if (Price && CharacterName && Description)
	{
		SetPriceText(PriceInfo);
		SetCharacterNameText(CharacterInfo);
		SetDescriptionText(DescriptionInfo);
		SetButtonFont(FInfo);
		SetButtonClickSFX(ButtonClickedSFX);
	}
}

void UCharacterButton::NativeOnInitialized() {
	Super::NativeOnInitialized();

	if (Button)
	{
		Button->OnClicked.AddDynamic(this, &UCharacterButton::HandleBackgroundClick);
		Button->OnHovered.AddDynamic(this, &UCharacterButton::HandleHoverSFX);
	}
}


void UCharacterButton::HandleBackgroundClick()
{
	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{
		if (CurrentButtonClicked) {

			I->PlaySound(CurrentButtonClicked);
		}
	}

	CodeButtonClick.Broadcast();

}

void UCharacterButton::HandleHoverSFX()
{
	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{
		if (ButtonHoverSFX) {
			I->PlaySound(ButtonHoverSFX);
		}
	}

}

void UCharacterButton::SetPriceText(FText text)
{
	Price->SetText(text);
}

void UCharacterButton::SetCharacterNameText(FText text)
{
	CharacterName->SetText(text);
}

void UCharacterButton::SetDescriptionText(FText text)
{
	Description->SetText(text);
}


void UCharacterButton::SetButtonFont(FSlateFontInfo FontInfo)
{
	if (Price && CharacterName && Description) {
		Price->SetFont(FontInfo);
		CharacterName->SetFont(FontInfo);
		Description->SetFont(FontInfo);
	}

}

void UCharacterButton::SetButtonClickSFX(USoundBase* SFX)
{
	CurrentButtonClicked = SFX;
}
