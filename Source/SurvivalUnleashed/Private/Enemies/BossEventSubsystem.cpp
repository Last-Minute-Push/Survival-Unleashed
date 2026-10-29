// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/BossEventSubsystem.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

void UBossEventSubsystem::ReportBossSpawned(AActor* InBoss, const FText& InBossName, float MaxHealth)
{
	if (bBossActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss '%s' spawned while '%s' is still active. Replacing it."),
			*InBossName.ToString(), *BossName.ToString());
	}

	Boss = InBoss;
	BossName = InBossName;
	BossMaxHealth = FMath::Max(MaxHealth, 1.f);
	BossHealth = BossMaxHealth;
	bBossActive = true;

	OnBossSpawned.Broadcast(BossName, BossMaxHealth);
}

void UBossEventSubsystem::ReportBossHealth(float CurrentHealth)
{
	if (!bBossActive)
	{
		return;
	}

	const float Clamped = FMath::Clamp(CurrentHealth, 0.f, BossMaxHealth);
	if (FMath::IsNearlyEqual(Clamped, BossHealth))
	{
		return;
	}

	BossHealth = Clamped;
	OnBossHealthChanged.Broadcast(BossHealth, BossMaxHealth);
}

void UBossEventSubsystem::ReportBossDied()
{
	if (!bBossActive)
	{
		return;
	}

	// Clear state before broadcasting so listeners already see IsBossActive() == false
	BossHealth = 0.f;
	bBossActive = false;
	Boss.Reset();

	OnBossDied.Broadcast();
}

bool UBossEventSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

#if !UE_BUILD_SHIPPING
namespace BossEventDebug
{
	static UBossEventSubsystem* Get(UWorld* World)
	{
		UBossEventSubsystem* Events = World ? World->GetSubsystem<UBossEventSubsystem>() : nullptr;
		if (!Events)
		{
			UE_LOG(LogTemp, Warning, TEXT("su.Boss: no BossEventSubsystem in this world. Run it during play."));
		}
		return Events;
	}

	static FAutoConsoleCommandWithWorldAndArgs FakeSpawn(
		TEXT("su.Boss.FakeSpawn"),
		TEXT("Starts a fake boss fight for testing listeners. Args: [MaxHealth=3000] [Name=Elder Treant]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
			{
				UBossEventSubsystem* Events = Get(World);
				if (!Events)
				{
					return;
				}

				const float MaxHealth = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 3000.f;

				// Names can have spaces, so everything after the health is the name
				FString Name;
				for (int32 i = 1; i < Args.Num(); ++i)
				{
					Name += (i > 1 ? TEXT(" ") : TEXT("")) + Args[i];
				}
				if (Name.IsEmpty())
				{
					Name = TEXT("Elder Treant");
				}

				Events->ReportBossSpawned(nullptr, FText::FromString(Name), MaxHealth);
			}));

	static FAutoConsoleCommandWithWorldAndArgs FakeDamage(
		TEXT("su.Boss.FakeDamage"),
		TEXT("Damages the active boss for testing listeners. Kills it at 0. Args: <Amount>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
			{
				UBossEventSubsystem* Events = Get(World);
				if (!Events || !Events->IsBossActive() || Args.Num() == 0)
				{
					return;
				}

				Events->ReportBossHealth(Events->GetBossHealth() - FCString::Atof(*Args[0]));
				if (Events->GetBossHealth() <= 0.f)
				{
					Events->ReportBossDied();
				}
			}));

	static FAutoConsoleCommandWithWorldAndArgs FakeDie(
		TEXT("su.Boss.FakeDie"),
		TEXT("Kills the active boss for testing listeners."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* World)
			{
				if (UBossEventSubsystem* Events = Get(World))
				{
					Events->ReportBossDied();
				}
			}));
}
#endif