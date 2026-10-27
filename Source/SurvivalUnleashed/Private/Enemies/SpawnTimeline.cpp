#include "Enemies/SpawnTimeline.h"
#include "Enemies/EnemyBase.h"

const FSpawnPhase* USpawnTimeline::GetPhaseAt(float RunTime) const
{
	const FSpawnPhase* Best = nullptr;
	for (const FSpawnPhase& Phase : Phases)
	{
		if (Phase.StartTime <= RunTime && (!Best || Phase.StartTime >= Best->StartTime))
		{
			Best = &Phase;
		}
	}
	return Best;
}

TSubclassOf<AEnemyBase> USpawnTimeline::PickEnemy(const FSpawnPhase& Phase)
{
	float Total = 0.f;
	for (const FSpawnEntry& Entry : Phase.Enemies)
	{
		if (Entry.EnemyClass) Total += Entry.Weight;
	}
	if (Total <= 0.f) return nullptr;

	float Roll = FMath::FRandRange(0.f, Total);
	for (const FSpawnEntry& Entry : Phase.Enemies)
	{
		if (!Entry.EnemyClass) continue;
		Roll -= Entry.Weight;
		if (Roll <= 0.f) return Entry.EnemyClass;
	}
	return Phase.Enemies.Last().EnemyClass; // float rounding fallback
}

void USpawnTimeline::GetAllEnemyClasses(TSet<TSubclassOf<AEnemyBase>>& OutClasses) const
{
	for (const FSpawnPhase& Phase : Phases)
		for (const FSpawnEntry& Entry : Phase.Enemies)
			if (Entry.EnemyClass) OutClasses.Add(Entry.EnemyClass);

	for (const FSpawnEvent& Event : Events)
		if (Event.EnemyClass) OutClasses.Add(Event.EnemyClass);
}
