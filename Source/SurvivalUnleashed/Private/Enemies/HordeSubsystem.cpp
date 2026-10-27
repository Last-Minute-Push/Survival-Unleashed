// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/HordeSubsystem.h"
#include "Enemies/EnemyBase.h"
#include "Debug/SUDebug.h"
#include "DrawDebugHelpers.h"

void UHordeSubsystem::RegisterEnemy(AEnemyBase* Enemy)
{
	if (Enemy)
	{
		Enemies.AddUnique(Enemy);
	}
}

void UHordeSubsystem::UnregisterEnemy(AEnemyBase* Enemy)
{
	Enemies.RemoveSingleSwap(Enemy);
}

void UHordeSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Empty each cell but keep its memory, so the grid doesn't reallocate every frame
	for (TPair<FIntPoint, TArray<AEnemyBase*>>& Cell : Grid)
	{
		Cell.Value.Reset();
	}

	for (AEnemyBase* Enemy : Enemies)
	{
		// Pooled enemies that are hidden or dead are inactive, so skip them
		if (IsValid(Enemy) && !Enemy->IsHidden() && !Enemy->IsDead())
		{
			Grid.FindOrAdd(ToCell(Enemy->GetActorLocation())).Add(Enemy);
		}
	}

#if ENABLE_DRAW_DEBUG
	if (SUDebug::IsAIDebugEnabled())
	{
		DrawDebugGrid();
	}
#endif
}

void UHordeSubsystem::DrawDebugGrid() const
{
#if ENABLE_DRAW_DEBUG
	for (const TPair<FIntPoint, TArray<AEnemyBase*>>& Cell : Grid)
	{
		const int32 Count = Cell.Value.Num();
		if (Count == 0)
		{
			continue; // cells keep their memory between frames, so empty ones stay in the map
		}

		const FVector Center((Cell.Key.X + 0.5f) * CellSize, (Cell.Key.Y + 0.5f) * CellSize, Cell.Value[0]->GetActorLocation().Z);

		// Green with 1 enemy, red at 8 or more: shows where the horde is packed tightest
		const float Crowding = FMath::Clamp((Count - 1) / 7.f, 0.f, 1.f);
		const FColor Color = FLinearColor::LerpUsingHSV(FLinearColor::Green, FLinearColor::Red, Crowding).ToFColor(true);

		DrawDebugBox(GetWorld(), Center, FVector(CellSize * 0.5f, CellSize * 0.5f, 5.f), Color, false, -1.f, 0, 3.f);
	}
#endif
}

void UHordeSubsystem::GetNeighbors(const FVector& Location, float Radius, TArray<AEnemyBase*>& OutNeighbors) const
{
	OutNeighbors.Reset();

	const FIntPoint Min = ToCell(Location - FVector(Radius, Radius, 0.f));
	const FIntPoint Max = ToCell(Location + FVector(Radius, Radius, 0.f));
	const float RadiusSq = Radius * Radius;

	for (int32 X = Min.X; X <= Max.X; ++X)
	{
		for (int32 Y = Min.Y; Y <= Max.Y; ++Y)
		{
			if (const TArray<AEnemyBase*>* Cell = Grid.Find(FIntPoint(X, Y)))
			{
				for (AEnemyBase* Enemy : *Cell)
				{
					if (IsValid(Enemy) && FVector::DistSquared2D(Enemy->GetActorLocation(), Location) <= RadiusSq)
					{
						OutNeighbors.Add(Enemy);
					}
				}
			}
		}
	}
}

AEnemyBase* UHordeSubsystem::FindNearestEnemy(const FVector& Location, float MaxRange) const
{
	const FIntPoint Center = ToCell(Location);
	const int32 MaxRing = FMath::CeilToInt(MaxRange / CellSize);

	AEnemyBase* Best = nullptr;
	float BestDistSq = MaxRange * MaxRange; // nothing beyond MaxRange counts

	// Search outward one square ring of cells at a time
	for (int32 Ring = 0; Ring <= MaxRing; ++Ring)
	{
		// Every cell in this ring is at least (Ring - 1) whole cells away from us,
			// so once we've found something closer than that, nothing farther out can win
		const float RingMinDist = FMath::Max(0, Ring - 1) * CellSize;
		if (Best && BestDistSq <= RingMinDist * RingMinDist)
		{
			break;
		}

		for (int32 X = -Ring; X <= Ring; ++X)
		{
			// The first and last columns are the full ring edge. Middle columns only touch it at the top and bottom
			const bool bEdgeColumn = (X == -Ring || X == Ring);
			const int32 Step = bEdgeColumn ? 1 : 2 * Ring;

			for (int32 Y = -Ring; Y <= Ring; Y += Step)
			{
				const TArray<AEnemyBase*>* Cell = Grid.Find(Center + FIntPoint(X, Y));
				if (!Cell)
				{
					continue;
				}

				for (AEnemyBase* Enemy : *Cell)
				{
					if (!IsTargetable(Enemy))
					{
						continue;
					}

					const float DistSq = FVector::DistSquared2D(Enemy->GetActorLocation(), Location);
					if (DistSq <= BestDistSq)
					{
						BestDistSq = DistSq;
						Best = Enemy;
					}
				}
			}
		}
	}

	return Best;
}

void UHordeSubsystem::GetEnemiesInRadius(const FVector& Location, float Radius, TArray<AEnemyBase*>& OutEnemies) const
{
	GetNeighbors(Location, Radius, OutEnemies);
	OutEnemies.RemoveAllSwap([](const AEnemyBase* Enemy) { return !IsTargetable(Enemy); });
}

void UHordeSubsystem::GetNearestEnemies(const FVector& Location, float MaxRange, int32 Count, TArray<AEnemyBase*>& OutEnemies) const
{
	GetEnemiesInRadius(Location, MaxRange, OutEnemies);

	OutEnemies.Sort([&Location](const AEnemyBase& A, const AEnemyBase& B)
		{
			return FVector::DistSquared2D(A.GetActorLocation(), Location) < FVector::DistSquared2D(B.GetActorLocation(), Location);
		});

	if (OutEnemies.Num() > Count)
	{
		OutEnemies.SetNum(FMath::Max(Count, 0), EAllowShrinking::No);
	}
}

bool UHordeSubsystem::IsTargetable(const AEnemyBase* Enemy)
{
	return IsValid(Enemy) && !Enemy->IsHidden() && !Enemy->IsDead();
}

FIntPoint UHordeSubsystem::ToCell(const FVector& Location)
{
	return FIntPoint(FMath::FloorToInt(Location.X / CellSize), FMath::FloorToInt(Location.Y / CellSize));
}

TStatId UHordeSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHordeSubsystem, STATGROUP_Tickables);
}

bool UHordeSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
