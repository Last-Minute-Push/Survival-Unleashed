// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PauseMenuBase.h"
#include "Actor/UC_Player.h"
#include "EngineUtils.h"
#include "UI/ButtonBase.h"
#include "../SurvivalUnleashed.h"

DEFINE_LOG_CATEGORY_STATIC(LogPauseMenuBase, Log, All);

UPauseMenuBase::UPauseMenuBase()
{
	ZOrderLayer = 2;
}

void UPauseMenuBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance()))
	{
		if (ContinueButton && QuitButton && OptionsButton)
		{
			ContinueButton->CodeButtonClick.AddDynamic(this, &UPauseMenuBase::ResumeGame);
			OptionsButton->CodeButtonClick.AddDynamic(this, &UPauseMenuBase::OptionsButtonPressed);
			QuitButton->CodeButtonClick.AddDynamic(GI, &UUGI_SUGameInstance::QuitToMenu);
		}
		else UE_LOG(LogPauseMenuBase, Error, TEXT("PlayButton and/or QuitButton not bound to the widget!"));
	}
	else UE_LOG(LogPauseMenuBase, Error, TEXT("Failed to cast the GameInstance because of an incorrect socketed GameInstance!"));
}

void UPauseMenuBase::ResumeGame()
{
	for (TActorIterator<AUC_Player> It(GetWorld()); It; ++It)
	{
		AUC_Player* Player = *It;
		if (Player)
		{
			Player->ResumeGame();
		}
	}
}

void UPauseMenuBase::OptionsButtonPressed()
{
	// Created and bound once, then reused every time Options opens
	if (!OptionHud)
	{
		OptionHud = CreateWidget<UOptionsMenuBase>(GetOwningPlayer(), OptionHUDClass);
		if (!OptionHud)
		{
			UE_LOG(LogPauseMenuBase, Error, TEXT("Could not create the Options menu; is OptionHUDClass set?"));
			return;
		}
		OptionHud->OnClosed.AddDynamic(this, &UPauseMenuBase::UnHidePauseButtons);
	}

	SetVisibility(ESlateVisibility::Collapsed); // hide the pause menu
	OptionHud->AddToViewport();
}

void UPauseMenuBase::UnHidePauseButtons()
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible); // a UserWidget's default visibility
}

void UPauseMenuBase::NativeDestruct()
{
	// Unpausing with Esc while Options is open removes the pause menu: close Options too,
	// and make sure the pause menu isn't still hidden the next time the game is paused
	if (OptionHud && OptionHud->IsInViewport())
	{
		OptionHud->RemoveFromParent();
	}
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	Super::NativeDestruct();
}