#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "Hornet.generated.h"

/** Fast, fragile swarm enemy. Flocks with other hornets (alignment + cohesion) while seeking the player. */
UCLASS()
class SURVIVALUNLEASHED_API AHornet : public AEnemyBase
{
	GENERATED_BODY()

public:
	AHornet();

	virtual void OnPoolAcquire_Implementation() override;

protected:
	virtual FVector GetDesiredDirection() const override;

	// Hornets within this distance count as part of the same swarm
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Flocking", meta = (ClampMin = "0"))
	float NeighborRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Flocking", meta = (ClampMin = "0"))
	float AlignmentWeight = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Flocking", meta = (ClampMin = "0"))
	float CohesionWeight = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Flocking", meta = (ClampMin = "0"))
	float WanderWeight = 0.3f;

	// How quickly the wander direction changes
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Flocking", meta = (ClampMin = "0"))
	float WanderFrequency = 1.5f;

	// 0.2 = speed drifts between 80% and 120% of MoveSpeed
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Flocking", meta = (ClampMin = "0", ClampMax = "1"))
	float SpeedVariation = 0.2f;

private:
	// Different per hornet so they don't all weave in sync
	float NoiseSeed = 0.f;

	mutable TArray<AEnemyBase*> FlockScratch;
};
