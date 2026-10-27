// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ObjectPoolSubsystem.generated.h"

/**
 *
 */

USTRUCT()
struct FActorPool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AActor>> Inactive;
};

UCLASS()
class SURVIVALUNLEASHED_API UObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Pooling", meta = (DeterminesOutputType = "ActorClass"))
	AActor* Acquire(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);

	template<typename T>
	T* AcquireAs(TSubclassOf<T> ActorClass, const FTransform& SpawnTransform)
	{
		return Cast<T>(Acquire(ActorClass, SpawnTransform));
	}

	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void Release(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void Prewarm(TSubclassOf<AActor> ActorClass, int32 Count);

	UFUNCTION(BlueprintPure, Category = "Pooling")
	int32 GetInactiveCount(TSubclassOf<AActor> ActorClass) const;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FActorPool> Pools;

	AActor* SpawnNew(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);
	static void SetActorActive(AActor* Actor, bool bActive);
};
