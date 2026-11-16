// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/FlowField.h"
#include "Framework/SUCollision.h"
#include "Debug/SUDebug.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

namespace
{
	// The 8 neighbors, with the cost of the step (diagonals are sqrt(2) cells long)
	struct FStep { int32 DX; int32 DY; float Cost; };
	constexpr FStep Steps[8] = {
		{ 1, 0, 1.f }, { -1, 0, 1.f }, { 0, 1, 1.f }, { 0, -1, 1.f },
		{ 1, 1, 1.41421356f }, { 1, -1, 1.41421356f }, { -1, 1, 1.41421356f }, { -1, -1, 1.41421356f } };

	struct FOpenCell
	{
		int32 Index;
		float Distance;
	};
}

AFlowField::AFlowField()
{
	PrimaryActorTick.bCanEverTick = true;

	// Only marks out the area; it never collides with anything
	Bounds = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
	Bounds->SetBoxExtent(FVector(5000.f, 5000.f, 200.f));
	Bounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Bounds->SetGenerateOverlapEvents(false);
	RootComponent = Bounds;
}

void AFlowField::BeginPlay()
{
	Super::BeginPlay();

	// The grid lines up with the world axes, so keep the box unrotated in the level
	const FVector Center = Bounds->GetComponentLocation();
	const FVector Extent = Bounds->GetScaledBoxExtent();
	Origin = FVector2D(Center.X - Extent.X, Center.Y - Extent.Y);
	FloorZ = Center.Z - Extent.Z;
	Width = FMath::Max(1, FMath::CeilToInt(2.f * Extent.X / CellSize));
	Height = FMath::Max(1, FMath::CeilToInt(2.f * Extent.Y / CellSize));

	const int32 Count = Width * Height;
	Blocked.SetNumZeroed(Count);
	Distance.Init(MAX_flt, Count);
	Flow.Init(FVector2f::ZeroVector, Count);
	LineOfSight.Init(-1, Count);

	// One overlap test per cell, once. The obstacles don't move.
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			Blocked[ToIndex(X, Y)] = TestBlocked(X, Y) ? 1 : 0;
		}
	}
}

void AFlowField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TimeSinceRebuild += DeltaTime;

	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		// Clamped onto the grid, so a player at the very edge still gets a field
		FIntPoint Cell = ToCell(Player->GetActorLocation());
		Cell.X = FMath::Clamp(Cell.X, 0, Width - 1);
		Cell.Y = FMath::Clamp(Cell.Y, 0, Height - 1);

		if ((Cell != Goal || bForceRebuild) && TimeSinceRebuild >= MinRebuildInterval)
		{
			Rebuild(Cell);
		}
	}

#if ENABLE_DRAW_DEBUG
	if (SUDebug::IsAIDebugEnabled())
	{
		DrawDebug();
	}
#endif
}

FIntPoint AFlowField::ToCell(const FVector& Location) const
{
	return FIntPoint(FMath::FloorToInt((Location.X - Origin.X) / CellSize), FMath::FloorToInt((Location.Y - Origin.Y) / CellSize));
}

FVector2D AFlowField::GetCellCenter(int32 X, int32 Y) const
{
	return Origin + FVector2D((X + 0.5f) * CellSize, (Y + 0.5f) * CellSize);
}

bool AFlowField::TestBlocked(int32 X, int32 Y) const
{
	// The cell grown by Clearance on every side, from the floor up to ProbeHeight.
	// Only the Obstacle channel, so the floor, enemies and pickups never count.
	const FVector2D Center2D = GetCellCenter(X, Y);
	const FVector Center(Center2D.X, Center2D.Y, FloorZ + ProbeHeight * 0.5f);
	const FVector HalfSize(CellSize * 0.5f + Clearance, CellSize * 0.5f + Clearance, ProbeHeight * 0.5f);

	return GetWorld()->OverlapAnyTestByObjectType(Center, FQuat::Identity,
		FCollisionObjectQueryParams(SUCollision::Obstacle), FCollisionShape::MakeBox(HalfSize));
}

bool AFlowField::CanStep(int32 X, int32 Y, int32 DX, int32 DY) const
{
	const int32 NX = X + DX;
	const int32 NY = Y + DY;
	if (!IsValidCell(NX, NY) || Blocked[ToIndex(NX, NY)])
	{
		return false;
	}
	if (DX != 0 && DY != 0)
	{
		return !Blocked[ToIndex(X + DX, Y)] && !Blocked[ToIndex(X, Y + DY)];
	}
	return true;
}

void AFlowField::Rebuild(const FIntPoint& NewGoal)
{
	Goal = NewGoal;
	TimeSinceRebuild = 0.f;
	bForceRebuild = false;

	for (float& D : Distance) { D = MAX_flt; }
	for (int8& L : LineOfSight) { L = -1; }

	// ---- Distance: Dijkstra outward from the player's cell. The goal is seeded even if it's blocked
	// (the player can stand closer to an obstacle than Clearance allows), but nothing walks into a blocked cell.
	const auto Closer = [](const FOpenCell& A, const FOpenCell& B) { return A.Distance < B.Distance; };
	TArray<FOpenCell> Open;
	Open.Reserve(Width * Height / 4);

	const int32 GoalIndex = ToIndex(Goal.X, Goal.Y);
	Distance[GoalIndex] = 0.f;
	Open.HeapPush({ GoalIndex, 0.f }, Closer);

	while (Open.Num() > 0)
	{
		FOpenCell Current;
		Open.HeapPop(Current, Closer, EAllowShrinking::No);
		if (Current.Distance > Distance[Current.Index])
		{
			continue; // a shorter way here was already found
		}

		const int32 X = Current.Index % Width;
		const int32 Y = Current.Index / Width;
		for (const FStep& Step : Steps)
		{
			if (!CanStep(X, Y, Step.DX, Step.DY))
			{
				continue;
			}
			const int32 Next = ToIndex(X + Step.DX, Y + Step.DY);
			const float NewDistance = Current.Distance + Step.Cost;
			if (NewDistance < Distance[Next])
			{
				Distance[Next] = NewDistance;
				Open.HeapPush({ Next, NewDistance }, Closer);
			}
		}
	}

	// ---- Flow: from each open cell, toward the neighbor that's closest to the player
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const int32 Index = ToIndex(X, Y);
			Flow[Index] = FVector2f::ZeroVector;
			if (Blocked[Index] || Distance[Index] == MAX_flt || Index == GoalIndex)
			{
				continue;
			}

			float Best = Distance[Index];
			for (const FStep& Step : Steps)
			{
				if (CanStep(X, Y, Step.DX, Step.DY))
				{
					const float D = Distance[ToIndex(X + Step.DX, Y + Step.DY)];
					if (D < Best)
					{
						Best = D;
						Flow[Index] = FVector2f(Step.DX, Step.DY).GetSafeNormal();
					}
				}
			}
		}
	}
}

bool AFlowField::HasLineOfSight(const FIntPoint& From) const
{
	const int32 FromIndex = ToIndex(From.X, From.Y);
	if (LineOfSight[FromIndex] >= 0)
	{
		return LineOfSight[FromIndex] == 1;
	}

	// Walk the grid cells on the line to the goal (Bresenham). The two end cells don't count: an enemy pushed up
	// against an obstacle, or a player standing next to one, can be in a blocked cell.
	int32 X = From.X;
	int32 Y = From.Y;
	const int32 DX = FMath::Abs(Goal.X - X);
	const int32 DY = -FMath::Abs(Goal.Y - Y);
	const int32 SX = X < Goal.X ? 1 : -1;
	const int32 SY = Y < Goal.Y ? 1 : -1;
	int32 Error = DX + DY;

	bool bClear = true;
	while (X != Goal.X || Y != Goal.Y)
	{
		const int32 E2 = 2 * Error;
		if (E2 >= DY) { Error += DY; X += SX; }
		if (E2 <= DX) { Error += DX; Y += SY; }

		if ((X != Goal.X || Y != Goal.Y) && Blocked[ToIndex(X, Y)])
		{
			bClear = false;
			break;
		}
	}

	LineOfSight[FromIndex] = bClear ? 1 : 0;
	return bClear;
}

FVector AFlowField::GetSeekDirection(const FVector& From, const FVector& PlayerLocation) const
{
	FVector Direct = PlayerLocation - From;
	Direct.Z = 0.f;
	Direct = Direct.GetSafeNormal();

	const FIntPoint Cell = ToCell(From);
	if (Goal.X < 0 || !IsValidCell(Cell.X, Cell.Y))
	{
		return Direct; // no field yet, or outside the grid
	}

	// Nothing in the way: straight at the player, exactly like before the flow field
	if (HasLineOfSight(Cell))
	{
		return Direct;
	}

	// Blend the flow of the four nearest cell centers by how close each is (bilinear), so enemies curve
	// smoothly instead of turning in 45-degree steps at cell borders
	const float FX = (From.X - Origin.X) / CellSize - 0.5f;
	const float FY = (From.Y - Origin.Y) / CellSize - 0.5f;
	const int32 X0 = FMath::FloorToInt(FX);
	const int32 Y0 = FMath::FloorToInt(FY);
	const float TX = FX - X0;
	const float TY = FY - Y0;

	FVector2f Sum = FVector2f::ZeroVector;
	const auto Add = [&](int32 X, int32 Y, float Weight)
	{
		if (IsValidCell(X, Y))
		{
			Sum += Flow[ToIndex(X, Y)] * Weight;
		}
	};
	Add(X0, Y0, (1.f - TX) * (1.f - TY));
	Add(X0 + 1, Y0, TX * (1.f - TY));
	Add(X0, Y0 + 1, (1.f - TX) * TY);
	Add(X0 + 1, Y0 + 1, TX * TY);

	if (!Sum.IsNearlyZero())
	{
		return FVector(Sum.X, Sum.Y, 0.f).GetSafeNormal();
	}

	// In a blocked or cut-off cell (pushed into an obstacle's Clearance): head for the open neighbor nearest the player
	float Best = MAX_flt;
	FVector2D Target = FVector2D::ZeroVector;
	for (const FStep& Step : Steps)
	{
		const int32 NX = Cell.X + Step.DX;
		const int32 NY = Cell.Y + Step.DY;
		if (IsValidCell(NX, NY) && !Blocked[ToIndex(NX, NY)] && Distance[ToIndex(NX, NY)] < Best)
		{
			Best = Distance[ToIndex(NX, NY)];
			Target = GetCellCenter(NX, NY);
		}
	}
	if (Best < MAX_flt)
	{
		return FVector(Target.X - From.X, Target.Y - From.Y, 0.f).GetSafeNormal();
	}
	return Direct; // somewhere the player can't be reached from
}

void AFlowField::RefreshArea(const FBox& Area)
{
	if (Width == 0)
	{
		return;
	}

	// Grown by Clearance plus a cell, so the cells whose grown test boxes touch the area are re-tested too
	const float Margin = Clearance + CellSize;
	const FIntPoint Min = ToCell(Area.Min - FVector(Margin, Margin, 0.f));
	const FIntPoint Max = ToCell(Area.Max + FVector(Margin, Margin, 0.f));

	for (int32 Y = FMath::Max(Min.Y, 0); Y <= FMath::Min(Max.Y, Height - 1); ++Y)
	{
		for (int32 X = FMath::Max(Min.X, 0); X <= FMath::Min(Max.X, Width - 1); ++X)
		{
			Blocked[ToIndex(X, Y)] = TestBlocked(X, Y) ? 1 : 0;
		}
	}
	bForceRebuild = true;
}

void AFlowField::DrawDebug() const
{
#if ENABLE_DRAW_DEBUG
	if (Goal.X < 0)
	{
		return;
	}

	const float Z = FloorZ + 10.f;
	for (int32 Y = FMath::Max(Goal.Y - DebugDrawRadius, 0); Y <= FMath::Min(Goal.Y + DebugDrawRadius, Height - 1); ++Y)
	{
		for (int32 X = FMath::Max(Goal.X - DebugDrawRadius, 0); X <= FMath::Min(Goal.X + DebugDrawRadius, Width - 1); ++X)
		{
			const int32 Index = ToIndex(X, Y);
			const FVector2D C = GetCellCenter(X, Y);
			const FVector Center(C.X, C.Y, Z);

			if (Blocked[Index])
			{
				DrawDebugBox(GetWorld(), Center, FVector(CellSize * 0.45f, CellSize * 0.45f, 1.f), FColor::Red, false, -1.f, 0, 2.f);
			}
			else if (!Flow[Index].IsNearlyZero())
			{
				// Green where an enemy would go straight at the player, yellow where it follows the field around something
				const bool bClear = LineOfSight[Index] == 1 || (LineOfSight[Index] == -1 && HasLineOfSight(FIntPoint(X, Y)));
				const FVector Tip = Center + FVector(Flow[Index].X, Flow[Index].Y, 0.f) * CellSize * 0.4f;
				DrawDebugDirectionalArrow(GetWorld(), Center, Tip, 20.f, bClear ? FColor::Green : FColor::Yellow, false, -1.f, 0, 2.f);
			}
		}
	}
#endif
}
