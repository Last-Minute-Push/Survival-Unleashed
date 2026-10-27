// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ButtonBase.h"
#include "Audio/Interfaces/AudioManagerInterface.h"
#include "../SurvivalUnleashed.h"

DEFINE_LOG_CATEGORY_STATIC(LogButtonBase, Log, All);

void UButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (Info)
	{
		SetButtonText(CodeInfo);
		SetButtonFont(FInfo);
		SetButtonClickSFX(ButtonClickedSFX);
	}
}

void UButtonBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (TheButton)
	{
		TheButton->OnClicked.AddDynamic(this, &UButtonBase::HandleBackgroundClick);
		TheButton->OnHovered.AddDynamic(this, &UButtonBase::HandleHoverSFX);
		
	}
}

void UButtonBase::HandleBackgroundClick()
{
	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{
		if (CurrentButtonClicked) {
			
			I->PlaySound(CurrentButtonClicked);
		}
		else
		{
			UE_LOG(LogButtonBase, Error, TEXT("The SFX for Button Hover is not set or null!"));
		}
	}
	else
	{
		UE_LOG(LogButtonBase, Error, TEXT("No Audio Library Instance has been found!"));
	}

	CodeButtonClick.Broadcast();
	
}

void UButtonBase::HandleHoverSFX()
{
	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{
		if (ButtonHoverSFX) {
			I->PlaySound(ButtonHoverSFX);
		}
		else
		{
			UE_LOG(LogButtonBase, Error, TEXT("The SFX for Button Hover is not set or null!"));
		}
	}
	else
	{
		UE_LOG(LogButtonBase, Error, TEXT("No Audio Library Instance has been found!"));
	}
}

void UButtonBase::SetButtonText(FText text)
{
	Info->SetText(text);
}

void UButtonBase::SetButtonFont(FSlateFontInfo FontInfo)
{
	Info->SetFont(FontInfo);
}

void UButtonBase::SetButtonClickSFX(USoundBase* SFX)
{
	CurrentButtonClicked = SFX;
}
