#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SUSaveGame.generated.h"

/** Permanent progress. Only UPROPERTY fields are written to disk. */
UCLASS()
class SURVIVALUNLEASHED_API USUSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static constexpr int32 LatestVersion = 2;

	UPROPERTY()
	int32 SaveVersion = LatestVersion;

	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	int32 UpgradePoints = 0;

	// Total points paid for ranks, so Refund All returns exactly what was spent
	UPROPERTY()
	int32 SpentPoints = 0;

	/** Stat name -> permanent rank (0-5) */
	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	TMap<FName, int32> PermanentRanks;

	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	int32 FocusIntelligence = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	int32 FocusBarbaric = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	int32 FocusModern = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Records")
	float BestTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Records")
	int32 BestKills = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Records")
	int32 BestLevel = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Records")
	float BestScore = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	float MasterVolume = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	float MusicVolume = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	float SoundVolume = 1.f;
};