// Fill out your copyright notice in the Description page of Project Settings.

#define LOCTEXT_NAMESPACE "MyNamespace"

#include "UI/PlayerHUDBase.h"
#include "Components/ProgressBar.h"
#include "Components/HorizontalBox.h"
#include "CommonNumericTextBlock.h"
#include "Components/Image.h"
#include "Audio/Interfaces/AudioManagerInterface.h"


void UPlayerHUDBase::Construct()
{
	Super::Construct();

	Timer = 0;
	UpdateTimer(Timer);
	RankArray.Reserve(0);
	RankIndex.Reserve(0);

	isPaused = false;
}

void UPlayerHUDBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!isPaused) {
		Timer += InDeltaTime;
		UpdateTimer(Timer);
	}
}

void UPlayerHUDBase::UpdateTimer(float TimePassed)
{
	FTimespan CurrentSeconds = FTimespan::FromSeconds(Timer);

	TimerText->SetText(FText::Format(FText::FromString(TEXT("{0} {1}")), FText::FromString("Timer: "), FText::AsTimespan(CurrentSeconds)));
	
}

FText UPlayerHUDBase::GetRankName(uint8 Stats)
{
	FText Result;
	switch (Stats)
	{
	case 0U:
		Result = FText::FromString("Attack : ");
		return Result ;
		break;
	case 1U:
		Result = FText::FromString("Health : ");
		return Result;
		break;
	case 2U:
		Result = FText::FromString("Defense : ");
		return Result;
		break;
	case 3U:
		Result = FText::FromString("Weapon Attack : ");
		return Result;
		break;
	case 4U:
		Result = FText::FromString("Attack Speed : ");
		return Result;
		break;
	case 5U:
		Result = FText::FromString("Reflect : ");
		return Result;
		break;
	case 6U:
		Result = FText::FromString("Dash Cooldown : ");
		return Result;
		break;
	case 7U:
		Result = FText::FromString("Dash Length : ");
		return Result;
		break;
	case 8U:
		Result = FText::FromString("Regen : ");
		return Result;
		break;
	case 9U:
		Result = FText::FromString("Life Steal : ");
		return Result;
		break;
	case 10U:
		Result = FText::FromString("EXP Gain : ");
		return Result;
		break;
	case 11U:
		Result = FText::FromString("Envasion : ");
		return Result;
		break;
	case 12U:
		Result = FText::FromString("Pick Up Range : ");
		return Result;
		break;
	case 13U:
		Result = FText::FromString("Projectile Amount : ");
		return Result;
		break;
	case 14U:
		Result = FText::FromString("Projectile Speed : ");
		return Result;
		break;
	case 15U:
		Result = FText::FromString("Heal Amount : ");
		return Result;
		break;

	}

	Result = FText::FromString("Error :");
	return Result;
}

void UPlayerHUDBase::SetHealth(float ratio)
{
	HealthBar->SetPercent(ratio);
	
}

void UPlayerHUDBase::SetExperience(float ratio, float Max)
{
	if (Max == 0)
	{
		EXPBar->SetPercent(0);
	}
	else
	EXPBar->SetPercent(ratio/Max);
}

void UPlayerHUDBase::SetLevelAndExp(float Level, float EXP)
{
	LevelCounter->SetText(FText::Format(FText::FromString(TEXT("{0} {1}")), FText::FromString("Level: "), FText::AsNumber(Level)));
	EXPGainCounter->SetText(FText::Format(FText::FromString(TEXT("{0} {1}")), FText::FromString("EXP: "), FText::AsNumber(EXP)));
}

void UPlayerHUDBase::SetPersonalBestTime(FText Time)
{
	PersonalBest->SetText(FText::Format(FText::FromString(TEXT("{0} {1}")), FText::FromString("Personal Best: "), Time));
}

void UPlayerHUDBase::PlayHurtFeedback()
{
	if(Fading)
	PlayAnimation(Fading);

	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{
		if (I->GetLibrary()->SFX_PlayerHurt) {

			I->PlaySound(I->GetLibrary()->SFX_PlayerHurt);
		}
		
	}
}

void UPlayerHUDBase::PlayHealFeedback()
{
	if (FadingHeal)
	PlayAnimation(FadingHeal);
}

void UPlayerHUDBase::SetStatsText(uint8 ChosenStats, int32 Ranks)
{

		/*FText CurrentString = FText::Format(FText::FromString(TEXT("{0} {1}")), GetRankName(ChosenStats), Ranks);
		
		

			for (int i = 1; i < 5; i++)
			{
				if (RankArray.IsValidIndex(i))
				{
					if (RankIndex[i] == ChosenStats) {
						RankArray[i] = CurrentString;
						UTextBlock* IndexText = Cast<UTextBlock>(StatBox->GetChildAt(i));
						IndexText->SetText(RankArray[i]);
						break;
					}
					else
					{
						RankArray[i] = CurrentString;
						RankIndex[i] = ChosenStats;
						UTextBlock* IndexText = Cast<UTextBlock>(StatBox->GetChildAt(i));
						IndexText->SetText(RankArray[i]);
						break;
					}
				}
			}
	*/
}

FText UPlayerHUDBase::GetCurrentTimer()
{
	return TimerText->GetText();
}
