#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "SUCheatManager.generated.h"

class ASpawnDirector;
class UDebugOverlaySubsystem;

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

	// DebugOverlay General: Displays the "General" Debug Overlay
	UFUNCTION(Exec)
	void DebugOverlay(const FString& Type);

	// Lists the overlays that are available
	UFUNCTION(Exec)
	void ListOverlays();

	// Sets the ZOrder of the Overlay
	UFUNCTION(Exec)
	void SetOverlayZOrder(int32 ZOrder);

	// Disables the Debug Overlay
	UFUNCTION(Exec)
	void DisableOverlay();

private:
	ASpawnDirector* FindSpawnDirector() const;
	UDebugOverlaySubsystem* FindDebugOverlaySubsystem() const;
	void Say(const FString& Message) const;
};
