// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enemies/EnemyBase.h"
#include "SpawnDirector.generated.h"

class UCapsuleComponent;

UCLASS()
class SURVIVALUNLEASHED_API ASpawnDirector : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASpawnDirector();

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void StopSpawning();

	UFUNCTION(BlueprintPure, Category = "Spawning")
	int32 GetActiveEnemyCount() const { return ActiveEnemyCount; }

	UFUNCTION(BlueprintPure, Category = "Spawning")
	float GetRunTime() const;

	// Only enabled while su.DebugAI is on; draws the spawn area and recycle distance every frame
	virtual void Tick(float DeltaSeconds) override;

	// ---- Testing (called by USUCheatManager)
	int32 SpawnEnemiesNow(TSubclassOf<AEnemyBase> Class, int32 Count);
	int32 KillAllEnemies();
	void SkipTime(float Seconds);
	void GetSpawnableClasses(TArray<TSubclassOf<AEnemyBase>>&OutClasses) const;

	// Fires for every enemy this director spawned — bind once instead of per enemy
	UPROPERTY(BlueprintAssignable, Category = "Spawning|Events")
	FOnEnemyHit OnAnyEnemyHit;

	UPROPERTY(BlueprintAssignable, Category = "Spawning|Events")
	FOnEnemyDied OnAnyEnemyDied;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// If set, the timeline decides what spawns and how many; EnemyClass and SpawnBatchSize are ignored.
	// MaxActiveEnemies is still the hard cap.
	UPROPERTY(EditAnywhere, Category = "Spawning")
	class USpawnTimeline* Timeline;

	// Pool size prewarmed for each enemy type found in the timeline
	UPROPERTY(EditAnywhere, Category = "Spawning", meta = (ClampMin = "0"))
	int32 TimelinePrewarmPerClass = 60;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<AEnemyBase> EnemyClass;

	UPROPERTY(EditAnywhere, Category = "Spawning", meta = (ClampMin = "0.05"))
	float SpawnInterval = 1.f;

	UPROPERTY(EditAnywhere, Category = "Spawning", meta = (ClampMin = "1"))
	int32 SpawnBatchSize = 3;

	UPROPERTY(EditAnywhere, Category = "Spawning", meta = (ClampMin = "1"))
	int32 MaxActiveEnemies = 150;

	// Extra distance beyond the screen edge
	UPROPERTY(EditAnywhere, Category = "Spawning", meta = (ClampMin = "0"))
	float SpawnMargin = 200.f;

	// Used if the screen corners can't be projected onto the ground
	UPROPERTY(EditAnywhere, Category = "Spawning", meta = (ClampMin = "0"))
	float FallbackSpawnRadius = 2000.f;

	// If true, spawning will start automatically when the game begins
	UPROPERTY(EditAnywhere, Category = "Spawning")
	bool bStartOnBeginPlay = true;

	// Actors with this tag (pillars, trees, boulders, logs, jungle wall) are never spawned on or inside
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	FName ObstacleTag = TEXT("Obstacle");

	// Dropped where an enemy dies. Uses EnemyData->HealthDropChance.
	UPROPERTY(EditAnywhere, Category = "Loot")
	TSubclassOf<AActor> HealthPickupClass;

	UPROPERTY(EditAnywhere, Category = "Loot", meta = (ClampMin = "0"))
	int32 HealthPickupPrewarm = 5;

	// Health pickups land this far from the enemy so they don't sit on top of its XP gem
	float HealthDropOffset = 50.f;

	// Space kept between a new enemy's capsule and any obstacle
	UPROPERTY(EditAnywhere, Category = "Spawning", meta = (ClampMin = "0"))
	float ClearancePadding = 20.f;

	// Enemies farther than this from the player (2D) are moved to a new off-screen spawn point.
	// Never less than the spawn ring plus a buffer, so fresh spawns aren't recycled.
	UPROPERTY(EditAnywhere, Category = "Spawning|Recycling", meta = (ClampMin = "0"))
	float RecycleDistance = 4000.f;
	
	UPROPERTY(EditAnywhere, Category = "Spawning|Recycling", meta = (ClampMin = "0.1"))
	float RecycleCheckInterval = 1.f;
	
	// Caps the work per check, so running away from the whole horde doesn't spike one frame
	UPROPERTY(EditAnywhere, Category = "Spawning|Recycling", meta = (ClampMin = "1"))
	int32 MaxRecyclesPerCheck = 20;

private:
	FTimerHandle SpawnTimer;
	FTimerHandle RecycleTimer;
	
	// Every enemy this director spawned that hasn't died; recycling walks this list
	TArray<TWeakObjectPtr<AEnemyBase>> ActiveEnemies;

	int32 ActiveEnemyCount = 0;
	float RunStartTime = 0.f;
	bool bRunStarted = false;
	TArray<float> NextEventTimes; // one per Timeline->Events entry

	// Called on a timer to spawn enemies
	void SpawnTick();

	// Spawns any timeline events that are due
	void ProcessEvents(float RunTime, const FVector& PlayerLocation);

	// Acquires one enemy from the pool at a ground point, facing the player, and hooks up its events
	AEnemyBase* SpawnEnemy(TSubclassOf<AEnemyBase> Class, const FVector& GroundLocation, const FVector& PlayerLocation);

	// Standing on GroundLocation (capsule bottom on the ground), facing the player
	FTransform GetSpawnTransform(TSubclassOf<AEnemyBase> Class, const FVector & GroundLocation, const FVector & PlayerLocation) const;
	
	// Moves enemies that fell too far behind to a new off-screen point. Not a death: no events, no loot.
	void RecycleCheck();

	float GetRecycleDistance(const FVector& PlayerLocation) const;

	// Returns a valid spawn location on the ground around the player, or false if none could be found
	// ClearanceRadius: the enemy's collision radius plus padding, so its body doesn't spawn clipped into an obstacle
	bool FindSpawnLocation(const FVector& PlayerLocation, float ClearanceRadius, FVector& OutLocation) const;
	bool IsSpawnPointClear(const FVector& GroundLocation, float ClearanceRadius) const;

	// The class's capsule size, read from its defaults (works without spawning one)
	static UCapsuleComponent* GetDefaultCapsule(TSubclassOf<AEnemyBase> Class);
	float GetSpawnClearance(TSubclassOf<AEnemyBase> Class) const;

	// The ground the camera can see, as 4 corners in order around the screen (at the player's height).
	// False if any corner's ray misses the ground; callers then use FallbackSpawnRadius.
	bool GetVisibleGround(const FVector& PlayerLocation, FVector2D(&OutCorners)[4]) const;

	// Distance from Origin along Dir to where it leaves the visible ground, or MAX_flt if it never does
	static float GetExitDistance(const FVector2D& Origin, const FVector2D& Dir, const FVector2D(&Corners)[4]);

	UFUNCTION()
	void HandleEnemyHit(AEnemyBase* Enemy, float Damage);

	UFUNCTION()
	void HandleEnemyDied(AEnemyBase* Enemy);

	void TryDropHealthPickup(const AEnemyBase* Enemy);
};
