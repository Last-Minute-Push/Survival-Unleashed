#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "SUCheatManager.generated.h"

class ASpawnDirector;

// Console commands for testing. Not created in shipping builds. "God" comes from UCheatManager.
UCLASS()
class SURVIVALUNLEASHED_API USUCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	// SpawnEnemies Crawler 20  (Type matches part of the class name; see ListEnemies)
	UFUNCTION(Exec)
	void SpawnEnemies(const FString& Type, int32 Count);

	// Lists the enemy types SpawnEnemies accepts
	UFUNCTION(Exec)
	void ListEnemies();

	// Kills every active enemy (drops loot, counts as kills)
	UFUNCTION(Exec)
	void KillEnemies();

	// SkipTime 120: jumps the spawn timeline ahead (phases, events, difficulty)
	UFUNCTION(Exec)
	void SkipTime(float Seconds);

	// GiveXP 50: same as collecting gems worth 50 XP
	UFUNCTION(Exec)
	void GiveXP(int32 Amount);

	// LevelUp 3: gains exactly that many levels (menus still open)
	UFUNCTION(Exec)
	void LevelUp(int32 Count);

private:
	ASpawnDirector* FindSpawnDirector() const;
	void Say(const FString& Message) const;
};
