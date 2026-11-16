// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/HUDBase.h"
#include "Components/WidgetSwitcher.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Framework/UGI_SUGameInstance.h"
#include "ResultsBase.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UResultsBase : public UHUDBase
{
	GENERATED_BODY()

	virtual void NativeOnInitialized() override;

protected:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButtonBase* QuitButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButtonBase* AgainButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* Timer_TextBlock;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* EnemyKilled_TextBlock;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* LevelReached_TextBlock;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* Score_TextBlock;
	

	UPROPERTY(EditDefaultsOnly)
	FTimerHandle RestTimer;
public:
	UResultsBase();

	void SetTextTimer(FText timer);
	void SetTextEnemyKilled(FText enemy);
	void SetTextlevelReached(FText level);
	void SetTextScore(FText score);

	
	UFUNCTION() void StopResultsMusic();
};
