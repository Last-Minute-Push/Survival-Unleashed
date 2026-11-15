// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "SporePuffball.generated.h"

class APoisonCloud;

/**
 * Specialist (07:00) that blocks off space. Waddles at the player; within 250 uu it stops and swells for 1.0 s,
 * then bursts into a poison cloud and dies (a self-burst drops no gem). Killed by the player, it still leaves a
 * cloud but drops its gem, so the player has to think about where it dies.
 * The swell is committed. As in, once it starts, it bursts even if the player walks away. Knocking it back during the
 * swell moves where the cloud lands. Freeze and Stun pause the swell.
 * The material reads the swell from AttackPose (0 -> 1) to inflate, shake and turn red.
 */
UCLASS()
class SURVIVALUNLEASHED_API ASporePuffball : public AEnemyBase
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;
	virtual void OnPoolAcquire_Implementation() override;

protected:
	virtual void BeginPlay() override;
	virtual FVector GetDesiredDirection() const override;

	UPROPERTY(EditDefaultsOnly, Category = "Puffball")
	TSubclassOf<APoisonCloud> CloudClass;

	// Clouds made up front, shared by every puffball (only made once)
	UPROPERTY(EditDefaultsOnly, Category = "Puffball", meta = (ClampMin = "0"))
	int32 CloudPrewarm = 8;

	UPROPERTY(EditDefaultsOnly, Category = "Puffball", meta = (ClampMin = "0"))
	float SwellTriggerRange = 250.f;

	UPROPERTY(EditDefaultsOnly, Category = "Puffball", meta = (ClampMin = "0.01"))
	float SwellTime = 1.f;

private:
	bool bSwelling = false;
	float SwellElapsed = 0.f;

	void Burst();

	// At the ground under the puffball
	void SpawnCloud(const FVector& ActorLocation) const;
};
