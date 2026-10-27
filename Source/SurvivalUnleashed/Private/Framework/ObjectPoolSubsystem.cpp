// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ObjectPoolSubsystem.h"
#include "Framework/Poolable.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogObjectPool, Log, All);

AActor* UObjectPoolSubsystem::Acquire(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform)
{
	if (!ActorClass)
	{
		return nullptr;
	}

	AActor* Actor = nullptr;
	FActorPool& Pool = Pools.FindOrAdd(ActorClass);

	// Skip anything that was destroyed while sitting in the pool
	while (!Actor && Pool.Inactive.Num() > 0)
	{
		AActor* Candidate = Pool.Inactive.Pop(EAllowShrinking::No);
		if (IsValid(Candidate))
		{
			Actor = Candidate;
		}
	}

	if (Actor)
	{
		Actor->SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
		SetActorActive(Actor, true);
	}
	else
	{
		Actor = SpawnNew(ActorClass, SpawnTransform);
		if (!Actor)
		{
			return nullptr;
		}
	}

	if (Actor->Implements<UPoolable>())
	{
		IPoolable::Execute_OnPoolAcquire(Actor);
	}

	return Actor;
}

void UObjectPoolSubsystem::Release(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	FActorPool& Pool = Pools.FindOrAdd(Actor->GetClass());
	if (Pool.Inactive.Contains(Actor))
	{
		UE_LOG(LogObjectPool, Warning, TEXT("%s was released twice"), *GetNameSafe(Actor));
		return;
	}

	if (Actor->Implements<UPoolable>())
	{
		IPoolable::Execute_OnPoolRelease(Actor);
	}

	SetActorActive(Actor, false);
	Pool.Inactive.Push(Actor);
}

void UObjectPoolSubsystem::Prewarm(TSubclassOf<AActor> ActorClass, int32 Count)
{
	if (!ActorClass)
	{
		return;
	}

	for (int32 i = 0; i < Count; ++i)
	{
		if (AActor* Actor = SpawnNew(ActorClass, FTransform::Identity))
		{
			SetActorActive(Actor, false);
			Pools.FindOrAdd(ActorClass).Inactive.Push(Actor);
		}
	}
}

int32 UObjectPoolSubsystem::GetInactiveCount(TSubclassOf<AActor> ActorClass) const
{
	const FActorPool* Pool = Pools.Find(ActorClass);
	return Pool ? Pool->Inactive.Num() : 0;
}

bool UObjectPoolSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

AActor* UObjectPoolSubsystem::SpawnNew(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UE_LOG(LogObjectPool, Log, TEXT("Spawning new %s"), *GetNameSafe(ActorClass));
	return GetWorld()->SpawnActor<AActor>(ActorClass, SpawnTransform, Params);
}

void UObjectPoolSubsystem::SetActorActive(AActor* Actor, bool bActive)
{
	Actor->SetActorHiddenInGame(!bActive);
	Actor->SetActorEnableCollision(bActive);
	Actor->SetActorTickEnabled(bActive);
}

