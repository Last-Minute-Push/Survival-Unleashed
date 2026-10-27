#include "Cheats/SUCheatManager.h"
#include "Enemies/SpawnDirector.h"
#include "Enemies/EnemyBase.h"
#include "Actor/UC_Player.h"
#include "Utility/PlayerStatsComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogSUCheats, Log, All);

void USUCheatManager::SpawnEnemies(const FString& Type, int32 Count)
{
#if UE_WITH_CHEAT_MANAGER
	ASpawnDirector* Director = FindSpawnDirector();
	if (!Director || Count <= 0) return;

	TArray<TSubclassOf<AEnemyBase>> Classes;
	Director->GetSpawnableClasses(Classes);
	for (const TSubclassOf<AEnemyBase>& Class : Classes)
	{
		if (Class->GetName().Contains(Type)) // case-insensitive by default
		{
			const int32 Spawned = Director->SpawnEnemiesNow(Class, Count);
			Say(FString::Printf(TEXT("Spawned %d of %d %s"), Spawned, Count, *Class->GetName()));
			return;
		}
	}
	Say(FString::Printf(TEXT("No enemy type matches \"%s\". Try ListEnemies."), *Type));
#endif
}

void USUCheatManager::ListEnemies()
{
#if UE_WITH_CHEAT_MANAGER
	if (ASpawnDirector* Director = FindSpawnDirector())
	{
		TArray<TSubclassOf<AEnemyBase>> Classes;
		Director->GetSpawnableClasses(Classes);
		if (Classes.IsEmpty())
		{
			Say(TEXT("No enemy types: the Spawn Director has no Timeline or EnemyClass set"));
			return;
		}
		for (const TSubclassOf<AEnemyBase>& Class : Classes)
		{
			Say(Class->GetName());
		}
	}
#endif
}

void USUCheatManager::KillEnemies()
{
#if UE_WITH_CHEAT_MANAGER
	if (ASpawnDirector* Director = FindSpawnDirector())
	{
		Say(FString::Printf(TEXT("Killed %d enemies"), Director->KillAllEnemies()));
	}
#endif
}

void USUCheatManager::SkipTime(float Seconds)
{
#if UE_WITH_CHEAT_MANAGER
	if (ASpawnDirector* Director = FindSpawnDirector())
	{
		Director->SkipTime(Seconds);
		Say(FString::Printf(TEXT("Run time is now %.0f s"), Director->GetRunTime()));
	}
#endif
}

void USUCheatManager::GiveXP(int32 Amount)
{
#if UE_WITH_CHEAT_MANAGER
	AUC_Player* Player = Cast<AUC_Player>(GetOuterAPlayerController()->GetPawn());
	if (!Player || !Player->GetPlayerStatsComp() || Amount <= 0) return;

	// Same call a gem pickup makes, so level-ups and menus behave exactly like real play
	Player->GetPlayerStatsComp()->UpdateExpANDLevel(Amount);
	Say(FString::Printf(TEXT("Gave %d XP"), Amount));
#endif
}

void USUCheatManager::LevelUp(int32 Count)
{
#if UE_WITH_CHEAT_MANAGER
	AUC_Player* Player = Cast<AUC_Player>(GetOuterAPlayerController()->GetPawn());
	if (!Player || !Player->GetPlayerStatsComp() || Count <= 0) return;

	UPlayerStatsComponent* Stats = Player->GetPlayerStatsComp();
	for (int32 i = 0; i < Count; ++i)
	{
		// TODO(Chris's API): XP still needed to reach the next level
		const int32 Needed = /* Stats->GetExpToNextLevel() */ 0;
		Stats->UpdateExpANDLevel(FMath::Max(Needed, 1));
	}
	Say(FString::Printf(TEXT("Leveled up %d times"), Count));
#endif
}

ASpawnDirector* USUCheatManager::FindSpawnDirector() const
{
	for (TActorIterator<ASpawnDirector> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	Say(TEXT("No Spawn Director in this level"));
	return nullptr;
}

void USUCheatManager::Say(const FString& Message) const
{
	GetOuterAPlayerController()->ClientMessage(Message);                    // full console (~)
	UE_LOG(LogSUCheats, Display, TEXT("%s"), *Message);                    // Output Log
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, Message); // on screen for 5 s
	}
}

