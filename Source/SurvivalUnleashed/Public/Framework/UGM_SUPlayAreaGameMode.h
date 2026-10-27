// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Actor/UC_Player.h"
#include "UI/ResultsBase.h"
#include "UI/LevelUpScreen.h"
#include "UGM_SUPlayAreaGameMode.generated.h"

class ASpawnDirector;
class AEnemyBase;

// SecondsLeft counts 3, 2, 1, then 0 = "SURVIVE!"
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCountdownTick, int32 /*SecondsLeft*/);
DECLARE_MULTICAST_DELEGATE(FOnRunStarted);

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API AUGM_SUPlayAreaGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:

	FOnCountdownTick OnCountdownTick;
	FOnRunStarted OnRunStarted;
	
	bool HasRunStarted() const { return bRunStarted; }
	int32 GetCountdownRemaining() const { return CountdownRemaining; } // for widgets created mid-countdown
	float GetRunTime() const; // 0 until the countdown ends; pauses with the game
protected:
	AUGM_SUPlayAreaGameMode();
	virtual void BeginPlay() override;
	AUC_Player* CurrentPlayer;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> HUDClassRS;
	UPROPERTY()
	UResultsBase* ResultScreenObject;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> HUDClassLU;
	UPROPERTY()
	ULevelUpScreen* LevelUpScreenObject;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> CGPCClass;
	UFUNCTION()
	void RemovePlayer();

	UFUNCTION()
	void PlayerLevelUp(); //add to view and make the player pause the game so they can make a choice
	UFUNCTION()
	void PlayerFocusUp(); //add to view and make the player pause the game so they can make a choice for focus stat then go to level up screen
	UFUNCTION()
	void ClearLevelUpScreen(); //remove from view once a non focus stat upgrade was chosen.
	UFUNCTION()
	void SwapFromFocustoLevel();


	// - Run tracking -
	UPROPERTY() TObjectPtr<ASpawnDirector> SpawnDirector;
	float RunStartTime = 0.f;
	int32 RunKills = 0;
	bool bRunEnded = false;

	UPROPERTY(EditDefaultsOnly, Category = "Run", meta = (ClampMin = "0"))
	int32 CountdownSeconds = 3;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UCountdownWidget> CountdownClass;
	
	FTimerHandle CountdownTimer;
	int32 CountdownRemaining = 0;
	bool bRunStarted = false;
	
	void CountdownTick();
	void StartRun();
	void SetPlayerControl(bool bEnabled);
	
	UFUNCTION() void HandleEnemyDied(AEnemyBase * Enemy);
	void RecordRun();
};
