// Fill out your copyright notice in the Description page of Project Settings.

#define LOCTEXT_NAMESPACE "MyNamespace"

#include "UI/PlayerHUDBase.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "CommonNumericTextBlock.h"
#include "Components/Image.h"
#include "Audio/Interfaces/AudioManagerInterface.h"


void UPlayerHUDBase::Construct()
{
	Super::Construct();

	Timer = 0;
	UpdateTimer(Timer);
	RankArray.Reserve(6);
	RankIndex.Reserve(6);

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

FText UPlayerHUDBase::GetRankName(EPlayerStats Stats)
{
	FText Result;
	switch (Stats)
	{
	case EPlayerStats::EPS_Attack:
		Result = FText::FromString("Attack : ");
		return Result ;
		break;
	case EPlayerStats::EPS_Health:
		Result = FText::FromString("Health : ");
		return Result;
		break;
	case EPlayerStats::EPS_Defense:
		Result = FText::FromString("Defense : ");
		return Result;
		break;
	case EPlayerStats::EPS_WeaponAttack:
		Result = FText::FromString("Weapon Attack : ");
		return Result;
		break;
	case EPlayerStats::EPS_AttackSpeed:
		Result = FText::FromString("Attack Speed : ");
		return Result;
		break;
	case EPlayerStats::EPS_Reflect:
		Result = FText::FromString("Reflect : ");
		return Result;
		break;
	case EPlayerStats::EPS_DashCooldown:
		Result = FText::FromString("Dash Cooldown : ");
		return Result;
		break;
	case EPlayerStats::EPS_DashLength:
		Result = FText::FromString("Dash Length : ");
		return Result;
		break;
	case EPlayerStats::EPS_Regen:
		Result = FText::FromString("Regen : ");
		return Result;
		break;
	case EPlayerStats::EPS_LifeSteal:
		Result = FText::FromString("Life Steal : ");
		return Result;
		break;
	case EPlayerStats::EPS_EXPGain:
		Result = FText::FromString("EXP Gain : ");
		return Result;
		break;
	case EPlayerStats::EPS_Evasion:
		Result = FText::FromString("Envasion : ");
		return Result;
		break;
	case EPlayerStats::EPS_PickupRange:
		Result = FText::FromString("Pick Up Range : ");
		return Result;
		break;
	case EPlayerStats::EPS_ProjectileAmount:
		Result = FText::FromString("Projectile Amount : ");
		return Result;
		break;
	case EPlayerStats::EPS_ProjectileSpeed:
		Result = FText::FromString("Projectile Speed : ");
		return Result;
		break;
	case EPlayerStats::EPS_HealAmount:
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

void UPlayerHUDBase::SetStatsText(EPlayerStats ChosenStats, int32 Ranks)
{
		FText CurrentString = FText::Format(FText::FromString(TEXT("{0} {1}")), GetRankName(ChosenStats), Ranks);

		if (RankArray.Num() != 5) {
			RankArray.Push(CurrentString);
			RankIndex.Push(ChosenStats);
			
			UTextBlock* IndexPip = Cast<UTextBlock>(StatsBox->GetChildAt(RankArray.Num()-1));
			IndexPip->SetText(CurrentString);
			
		}
			
}

FText UPlayerHUDBase::GetCurrentTimer()
{
	return TimerText->GetText();
}
