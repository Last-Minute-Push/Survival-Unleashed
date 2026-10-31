#pragma once

#include "CoreMinimal.h"
#include "UI/HUDBase.h"
#include "CountdownWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

// Big "3, 2, 1, SURVIVE!" in the middle of the screen at the start of a run.
// Created by the PlayArea game mode; listens to its countdown and removes itself when the run starts.
UCLASS(Abstract)
class SURVIVALUNLEASHED_API UCountdownWidget : public UHUDBase
{
	GENERATED_BODY()

public:
	UCountdownWidget();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountdownText;

	// Optional: a short scale-and-fade "pop" played on every number
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> PopAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Countdown")
	FText GoText = INVTEXT("SURVIVE!");

	// How long GoText stays up before the widget removes itself
	UPROPERTY(EditDefaultsOnly, Category = "Countdown", meta = (ClampMin = "0"))
	float GoDisplayTime = 1.f;

private:
	void HandleTick(int32 SecondsLeft);

	FDelegateHandle TickHandle;
	FTimerHandle HideTimer;
};