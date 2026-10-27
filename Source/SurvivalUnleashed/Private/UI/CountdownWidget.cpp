#include "UI/CountdownWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "Framework/UGM_SUPlayAreaGameMode.h"
#include "TimerManager.h"

void UCountdownWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AUGM_SUPlayAreaGameMode* GM = GetWorld()->GetAuthGameMode<AUGM_SUPlayAreaGameMode>();
	if (!GM || GM->HasRunStarted())
	{
		RemoveFromParent(); // nothing to count down
		return;
	}

	TickHandle = GM->OnCountdownTick.AddUObject(this, &UCountdownWidget::HandleTick);
	HandleTick(GM->GetCountdownRemaining()); // show the current number right away
}

void UCountdownWidget::NativeDestruct()
{
	if (AUGM_SUPlayAreaGameMode* GM = GetWorld()->GetAuthGameMode<AUGM_SUPlayAreaGameMode>())
	{
		GM->OnCountdownTick.Remove(TickHandle);
	}
	GetWorld()->GetTimerManager().ClearTimer(HideTimer);
	Super::NativeDestruct();
}

void UCountdownWidget::HandleTick(int32 SecondsLeft)
{
	CountdownText->SetText(SecondsLeft > 0 ? FText::AsNumber(SecondsLeft) : GoText);

	if (PopAnim)
	{
		PlayAnimation(PopAnim);
	}

	if (SecondsLeft <= 0)
	{
		GetWorld()->GetTimerManager().SetTimer(HideTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				RemoveFromParent();
			}), GoDisplayTime, false);
	}
}