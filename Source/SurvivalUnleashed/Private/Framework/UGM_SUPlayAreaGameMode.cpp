// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/UGM_SUPlayAreaGameMode.h"
#include "Framework/UGI_SUGameInstance.h"
#include "EngineUtils.h"
#include "Framework/UPC_SUPlayAreaPlayerController.h"
#include "Actor/UC_Player.h"
#include "Enemies/SpawnDirector.h"
#include "Core/SUSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "UI/CountdownWidget.h"

AUGM_SUPlayAreaGameMode::AUGM_SUPlayAreaGameMode()
{
	PlayerControllerClass = AUPC_SUPlayAreaPlayerController::StaticClass();
}

void AUGM_SUPlayAreaGameMode::BeginPlay() {
	Super::BeginPlay();
	for (TActorIterator<AUC_Player> It(GetWorld()); It; ++It)
	{
		AUC_Player* Player = *It;
		if (Player)
		{
			CurrentPlayer = Player;
			CurrentPlayer->OPL.AddDynamic(this, &AUGM_SUPlayAreaGameMode::RemovePlayer);

		}
	}

	for (TActorIterator<AActor> It(GetWorld(), PlayerControllerClass); It; ++It)
	{
		AActor* Actor = *It;
		APlayerController* PlayerController = Cast<APlayerController>(Actor);
		ResultScreenObject = CreateWidget<UResultsBase>(PlayerController, HUDClassRS);
		LevelUpScreenObject = CreateWidget<ULevelUpScreen>(PlayerController, HUDClassLU);
		if (LevelUpScreenObject) {
			LevelUpScreenObject->CurrentPlayer = CurrentPlayer;
			LevelUpScreenObject->CurrentStatsComp = CurrentPlayer->GetPlayerStatsComp();
			CurrentPlayer->CurrentLvlScreen = LevelUpScreenObject;
			CurrentPlayer->GetPlayerStatsComp()->CurrentLvlScreen = LevelUpScreenObject;
			LevelUpScreenObject->CurrentStatsComp->OPLU.AddDynamic(this, &AUGM_SUPlayAreaGameMode::PlayerLevelUp);
			LevelUpScreenObject->CurrentStatsComp->OPFU.AddDynamic(this, &AUGM_SUPlayAreaGameMode::PlayerFocusUp);
			LevelUpScreenObject->CurrentStatsComp->OPLUE.AddDynamic(this, &AUGM_SUPlayAreaGameMode::ClearLevelUpScreen);
			LevelUpScreenObject->CurrentStatsComp->OPFUE.AddDynamic(this, &AUGM_SUPlayAreaGameMode::SwapFromFocustoLevel);
		}
	}

	// Countdown last, so the director is found before the run can start
	SetPlayerControl(false);
	CountdownRemaining = CountdownSeconds;
	if (CountdownClass && CountdownRemaining > 0)
	{
		if (UCountdownWidget* Countdown = CreateWidget<UCountdownWidget>(GetWorld()->GetFirstPlayerController(), CountdownClass))
		{
			Countdown->AddToViewport();
		}
	}
	if (CountdownRemaining <= 0)
	{
		StartRun();
	}
	else
	{
		OnCountdownTick.Broadcast(CountdownRemaining);
		GetWorldTimerManager().SetTimer(CountdownTimer, this, &AUGM_SUPlayAreaGameMode::CountdownTick, 1.f, true);
	}

	SpawnDirector = Cast<ASpawnDirector>(UGameplayStatics::GetActorOfClass(this, ASpawnDirector::StaticClass()));
	if (SpawnDirector)
	{
		SpawnDirector->OnAnyEnemyDied.AddDynamic(this, &AUGM_SUPlayAreaGameMode::HandleEnemyDied);
	}
}

void AUGM_SUPlayAreaGameMode::CountdownTick()
{
	--CountdownRemaining;
	OnCountdownTick.Broadcast(CountdownRemaining); // 0 = "SURVIVE!"
	if (CountdownRemaining <= 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimer);
		StartRun();
	}
}

void AUGM_SUPlayAreaGameMode::StartRun()
{
	bRunStarted = true;
	RunStartTime = GetWorld()->GetTimeSeconds();
	SetPlayerControl(true);
	if (SpawnDirector)
	{
		SpawnDirector->StartSpawning();
	}
	OnRunStarted.Broadcast();
}

void AUGM_SUPlayAreaGameMode::SetPlayerControl(bool bEnabled)
{
	APlayerController* PC = CurrentPlayer ? Cast<APlayerController>(CurrentPlayer->GetController()) : nullptr;
	if (!PC)
	{
		return;
	}
	if (bEnabled)
	{
		CurrentPlayer->EnableInput(PC);
	}
	else
	{
		CurrentPlayer->DisableInput(PC);
	}
}

float AUGM_SUPlayAreaGameMode::GetRunTime() const
{
	return bRunStarted ? GetWorld()->GetTimeSeconds() - RunStartTime : 0.f;
}

void AUGM_SUPlayAreaGameMode::RemovePlayer()
{
	if (bRunEnded) return; // a run can only end once
	bRunEnded = true;

	if (SpawnDirector) SpawnDirector->StopSpawning();
	RecordRun(); // before the results screen is added, so it can read the results

	if (ResultScreenObject) {
		if (UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance()))
		{
			FRunResults TempResults;
			TempResults.TimeSurvived = GetRunTime();
			TempResults.Kills = RunKills;
			TempResults.LevelReached = CurrentPlayer->GetPlayerStatsComp()->GetCurrLevel();
			if (CurrentPlayer->PlayerHUD->Timer > GI->GetLastRunResults().PreviousBestTime) {

				ResultScreenObject->SetTextTimer(FText::FromString(CurrentPlayer->PlayerHUD->GetCurrentTimer().ToString() + FString(TEXT(" (New Personal Best!)"))));
			}
			else {
				ResultScreenObject->SetTextTimer(CurrentPlayer->PlayerHUD->GetCurrentTimer());
			}
			if (RunKills > GI->GetLastRunResults().PreviousBestKills) {
				FText EnemyCount = FText::FromString(FString(TEXT("Enemies Killed: ") + FString(FString::FromInt(RunKills)) + FString(TEXT(" (New Personal Best!)"))));
				ResultScreenObject->SetTextEnemyKilled(EnemyCount);
			}
			else {
				FText EnemyCount = FText::FromString(FString(TEXT("Enemies Killed: ") + FString(FString::FromInt(RunKills))));
				ResultScreenObject->SetTextEnemyKilled(EnemyCount);
			}
			if (CurrentPlayer->GetPlayerStatsComp()->GetCurrLevel() > GI->GetLastRunResults().PreviousBestLevel) {
				FText LevelReached = FText::FromString(FString(TEXT("Your character reached level ") + FString(FString::FromInt(CurrentPlayer->GetPlayerStatsComp()->GetCurrLevel()) + FString(TEXT("!")) + FString(TEXT(" (New Personal Best!)")))));
				ResultScreenObject->SetTextlevelReached(LevelReached);
			}
			else {
				FText LevelReached = FText::FromString(FString(TEXT("Your character reached level ") + FString(FString::FromInt(CurrentPlayer->GetPlayerStatsComp()->GetCurrLevel()) + FString(TEXT("!")))));
				ResultScreenObject->SetTextlevelReached(LevelReached);
			}
			if (TempResults.CalculatePoints() > GI->GetLastRunResults().PreviousBestScore) {
				FText ScoreGot = FText::FromString(FString(TEXT("Score: ") + FString(FString::FromInt(TempResults.CalculatePoints())) + FString(TEXT(" (New Personal Best!)"))));
				ResultScreenObject->SetTextScore(ScoreGot);
			}
			else {
				FText ScoreGot = FText::FromString(FString(TEXT("Score: ") + FString(FString::FromInt(TempResults.CalculatePoints()))));
				ResultScreenObject->SetTextScore(ScoreGot);
			}


			ResultScreenObject->AddToViewport();
			CurrentPlayer->PlayerLost();
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(ResultScreenObject->TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
			ResultScreenObject->GetOwningPlayer()->SetInputMode(InputModeData);
			ResultScreenObject->GetOwningPlayer()->bShowMouseCursor = true;
		}
	}
}

void AUGM_SUPlayAreaGameMode::PlayerLevelUp()
{
	CurrentPlayer->LevelUpPause();
	LevelUpScreenObject->SetFourUpgrades();
	LevelUpScreenObject->EnableLevelUpScreen();
	LevelUpScreenObject->AddToViewport();
	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(LevelUpScreenObject->TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	LevelUpScreenObject->GetOwningPlayer()->SetInputMode(InputModeData);
	LevelUpScreenObject->GetOwningPlayer()->bShowMouseCursor = true;
}

void AUGM_SUPlayAreaGameMode::PlayerFocusUp()
{
	CurrentPlayer->LevelUpPause();
	LevelUpScreenObject->EnabledFocusScreen();
	LevelUpScreenObject->AddToViewport();
	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(LevelUpScreenObject->TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	LevelUpScreenObject->GetOwningPlayer()->SetInputMode(InputModeData);
	LevelUpScreenObject->GetOwningPlayer()->bShowMouseCursor = true;
}

void AUGM_SUPlayAreaGameMode::ClearLevelUpScreen()
{
	CurrentPlayer->LevelUpResume();
	FInputModeGameOnly InputModeData;
	LevelUpScreenObject->GetOwningPlayer()->SetInputMode(InputModeData);
	LevelUpScreenObject->GetOwningPlayer()->bShowMouseCursor = false;
	LevelUpScreenObject->RemoveFromParent();
	//should be fine? gonna edit this when I get player so it works as expected.
}

void AUGM_SUPlayAreaGameMode::SwapFromFocustoLevel()
{
	LevelUpScreenObject->SetFourUpgrades();
	LevelUpScreenObject->EnableLevelUpScreen();
}

void AUGM_SUPlayAreaGameMode::HandleEnemyDied(AEnemyBase* Enemy)
{
	if (!bRunEnded) ++RunKills;
}

void AUGM_SUPlayAreaGameMode::RecordRun()
{
	FRunResults Results;
	Results.TimeSurvived = GetRunTime();
	Results.Kills = RunKills;
	Results.LevelReached = CurrentPlayer->GetPlayerStatsComp()->GetCurrLevel();
	// TODO (Chris): player's level once XP and level-up are in 
	// TASK DONE, not sure where XP goes here but ya can always get level here!
	// BossesKilled / ElitesKilled: filled in once bosses (B) and elites (C) exist

	UE_LOG(LogTemp, Log, TEXT("Run ended: %.1f s, %d kills"), Results.TimeSurvived, Results.Kills);

	if (UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance()))
	{
		GI->RecordRunResults(Results);
	}
}