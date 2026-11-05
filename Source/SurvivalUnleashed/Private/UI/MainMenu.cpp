// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainMenu.h"
#include "UI/ButtonBase.h"
#include "UI/OptionsMenuBase.h"
#include "UI/PermaUpgradeMenu.h"
#include "Audio/Interfaces/AudioManagerInterface.h"
#include"../SurvivalUnleashed.h"

DEFINE_LOG_CATEGORY_STATIC(LogMainMenu, Log, All);

void UMainMenu::NativeOnInitialized()
{
	Super::NativeOnInitialized();


	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{
		I->PlayMusic(I->GetLibrary()->BGM_MainMenuSong);
	}

	if (UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance()))
	{
		if (PlayButton && QuitButton)
		{

			PlayButton->CodeButtonClick.AddDynamic(this, &UMainMenu::EndMainMenuMusic);
			PlayButton->CodeButtonClick.AddDynamic(GI, &UUGI_SUGameInstance::LoadFirstLevel);
			UpgradeButton->CodeButtonClick.AddDynamic(this, &UMainMenu::UpgradeButtonPressed);
			SettingsButton->CodeButtonClick.AddDynamic(this, &UMainMenu::SettingsButtonPressed);
			QuitButton->CodeButtonClick.AddDynamic(GI, &UUGI_SUGameInstance::QuitTheGame);
		}
		else UE_LOG(LogMainMenu, Error, TEXT("PlayButton and/or QuitButton not bound to the widget!"));
	}
	else UE_LOG(LogMainMenu, Error, TEXT("Failed to cast the GameInstance because of an incorrect socketed GameInstance!"));

	ZOrderLayer = 0;
}

void UMainMenu::SettingsButtonPressed()
{
	if (!OptionHud)
	{
		OptionHud = CreateWidget<UOptionsMenuBase>(GetOwningPlayer(), OptionHUDClass);
		if (!OptionHud)
		{
			UE_LOG(LogMainMenu, Error, TEXT("Could not create the Options menu; is OptionHUDClass set?"));
			return;
		}
		OptionHud->SetIsFocusable(true);
		OptionHud->OnClosed.AddDynamic(this, &UMainMenu::UnHideMainMenuButtons);
	}

	SetVisibility(ESlateVisibility::Collapsed); // hide the main menu
	OptionHud->AddToViewport();
}

void UMainMenu::UpgradeButtonPressed()
{
	if (!UpgradeHud)
	{
		UpgradeHud = CreateWidget<UPermaUpgradeMenu>(GetOwningPlayer(), UpgradeHUDClass);
		if (!UpgradeHud)
		{
			UE_LOG(LogMainMenu, Error, TEXT("Could not create the Upgrade menu; is UpgradeHUDClass set?"));
			return;
		}
		UpgradeHud->SetIsFocusable(true);
		UpgradeHud->OnUpgradeClose.AddDynamic(this, &UMainMenu::UnHideMainMenuButtons);
	}

	SetVisibility(ESlateVisibility::Collapsed); // hide the main menu
	UpgradeHud->AddToViewport();
}

void UMainMenu::EndMainMenuMusic()
{
	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{
		if (I->GetLibrary()->BGM_MainMenuSong) {
			I->StopMusic();
		}
		else
		{
			UE_LOG(LogMainMenu, Error, TEXT("BGM_TestSong4 is not set or null!"))
		}
	}
	else
	{
		UE_LOG(LogMainMenu, Error, TEXT("No Audio Library Instance has been found!"))
	}
}

void UMainMenu::UnHideMainMenuButtons()
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}
