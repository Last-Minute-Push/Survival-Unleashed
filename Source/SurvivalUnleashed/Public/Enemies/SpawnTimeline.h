#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpawnTimeline.generated.h"

class AEnemyBase;

/** One enemy type the director can pick, and how often relative to the others in the phase. */
USTRUCT(BlueprintType)
struct FSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AEnemyBase> EnemyClass;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0"))
	float Weight = 1.f;
};

/** From StartTime on, keep TargetActive enemies alive, chosen from Enemies by weight. */
USTRUCT(BlueprintType)
struct FSpawnPhase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0", Units = "s"))
	float StartTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0"))
	int32 TargetActive = 20;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "1"))
	int32 BatchSize = 3;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<FSpawnEntry> Enemies;
};

/** A group spawned together at one ring point at a set time (swarms now, bosses later). */
USTRUCT(BlueprintType)
struct FSpawnEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AEnemyBase> EnemyClass;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0", Units = "s"))
	float Time = 60.f;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "1"))
	int32 Count = 1;

	/** 0 = happens once. Otherwise repeats every RepeatInterval seconds after Time. */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0", Units = "s"))
	float RepeatInterval = 0.f;

	/** Spread around the group's ring point so the group doesn't stack on one spot. */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0"))
	float ClusterRadius = 150.f;
};

/** What spawns when during a run. Read by ASpawnDirector. */
UCLASS(BlueprintType)
class SURVIVALUNLEASHED_API USpawnTimeline : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Timeline")
	TArray<FSpawnPhase> Phases;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	TArray<FSpawnEvent> Events;

	/** Enemy health multiplier added per minute of run time (0.08 = +8%/min). Bosses ignore it. */
	UPROPERTY(EditAnywhere, Category = "Difficulty", meta = (ClampMin = "0"))
	float HealthScalePerMinute = 0.08f;

	/** Enemy damage multiplier added per minute of run time. */
	UPROPERTY(EditAnywhere, Category = "Difficulty", meta = (ClampMin = "0"))
	float DamageScalePerMinute = 0.04f;

	/** The latest phase whose StartTime has passed, or nullptr if none has started. */
	const FSpawnPhase* GetPhaseAt(float RunTime) const;

	/** Weighted random pick from a phase's enemy list. */
	static TSubclassOf<AEnemyBase> PickEnemy(const FSpawnPhase& Phase);

	/** Every enemy class used anywhere in the timeline (for prewarming the pool). */
	void GetAllEnemyClasses(TSet<TSubclassOf<AEnemyBase>>& OutClasses) const;
};
