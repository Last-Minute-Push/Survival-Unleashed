// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ResultsBase.h"
#include "UI/ButtonBase.h"
#include "Audio/Interfaces/AudioManagerInterface.h"

UResultsBase::UResultsBase()
{
	ZOrderLayer = 2;
}

void UResultsBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UUGI_SUGameInstance* GameInsta = Cast<UUGI_SUGameInstance>(GetGameInstance());

	if (GameInsta)
	{
		QuitButton->CodeButtonClick.AddDynamic(this, &UResultsBase::StopResultsMusic);
		QuitButton->CodeButtonClick.AddDynamic(GameInsta, &UUGI_SUGameInstance::QuitToMenu);
		AgainButton->CodeButtonClick.AddDynamic(this, &UResultsBase::StopResultsMusic);
		AgainButton->CodeButtonClick.AddDynamic(GameInsta, &UUGI_SUGameInstance::ReloadLevel);
	}
	else
	{
		//UE_LOG(Game, Log, TEXT(">>>>Error: Wrong Game Instance"))
	}
}

void UResultsBase::SetTextTimer(FText timer)
{
	Timer_TextBlock->SetText(timer);
}

void UResultsBase::SetTextEnemyKilled(FText enemy)
{
	EnemyKilled_TextBlock->SetText(enemy);
}

void UResultsBase::SetTextlevelReached(FText level)
{
	LevelReached_TextBlock->SetText(level);
}

void UResultsBase::SetTextScore(FText score)
{
	Score_TextBlock->SetText(score);
}

void UResultsBase::StopResultsMusic()
{
	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{
		I->StopMusic();
	}
}
