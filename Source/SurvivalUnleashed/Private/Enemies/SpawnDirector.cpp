// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/SpawnDirector.h"
#include "Enemies/SpawnTimeline.h"
#include "Framework/ObjectPoolSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Debug/SUDebug.h"
#include "Enemies/EnemyData.h"
#include "HAL/IConsoleManager.h"
#include "Framework/SUCollision.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpawnDirector, Log, All);

static TAutoConsoleVariable<bool> CVarForceHealthDrops(
	TEXT("su.ForceHealthDrops"), false,
	TEXT("Every enemy death drops a health pickup"),
	ECVF_Cheat);

// Sets default values
ASpawnDirector::ASpawnDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // SpawnTick turns it on while su.DebugAI is set
}

// Called when the game starts or when spawned
void ASpawnDirector::BeginPlay()
{
	Super::BeginPlay();

	if (!EnemyClass && !Timeline)
	{
		UE_LOG(LogSpawnDirector, Warning, TEXT("%s has no Timeline or EnemyClass set"), *GetName());
		return;
	}

	// Spawn the whole pool up front so there are no hitches mid-run
	if (UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
	{
		if (Timeline)
		{
			TSet<TSubclassOf<AEnemyBase>> Classes;
			Timeline->GetAllEnemyClasses(Classes);
			for (const TSubclassOf<AEnemyBase>& Class : Classes)
			{
				Pool->Prewarm(Class, TimelinePrewarmPerClass);
			}
		}
		else
		{
			Pool->Prewarm(EnemyClass, MaxActiveEnemies);
		}

		if (HealthPickupClass)
		{
			Pool->Prewarm(HealthPickupClass, HealthPickupPrewarm);
		}
	}

	if (bStartOnBeginPlay)
	{
		StartSpawning();
	}
}

void ASpawnDirector::StartSpawning()
{
	if (!EnemyClass && !Timeline)
	{
		return;
	}

	if (!bRunStarted)
	{
		bRunStarted = true;
		RunStartTime = GetWorld()->GetTimeSeconds();

		NextEventTimes.Reset();
		if (Timeline)
		{
			for (const FSpawnEvent& Event : Timeline->Events)
			{
				NextEventTimes.Add(Event.Time);
			}
		}
	}

	GetWorldTimerManager().SetTimer(SpawnTimer, this, &ASpawnDirector::SpawnTick, SpawnInterval, true);
	GetWorldTimerManager().SetTimer(RecycleTimer, this, &ASpawnDirector::RecycleCheck, RecycleCheckInterval, true);
}

void ASpawnDirector::StopSpawning()
{
	GetWorldTimerManager().ClearTimer(SpawnTimer);
	GetWorldTimerManager().ClearTimer(RecycleTimer);
}

void ASpawnDirector::SpawnTick()
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (!Player || !Pool)
	{
		return;
	}

#if ENABLE_DRAW_DEBUG
	// Picks up su.DebugAI changes within one spawn interval
	SetActorTickEnabled(SUDebug::IsAIDebugEnabled());
#endif

	const FVector PlayerLocation = Player->GetActorLocation();

	// Without a timeline this is the original fixed behavior
	int32 Target = MaxActiveEnemies;
	int32 BatchSize = SpawnBatchSize;
	const FSpawnPhase* Phase = nullptr;

	if (Timeline)
	{
		const float RunTime = GetRunTime();
		ProcessEvents(RunTime, PlayerLocation);

		Phase = Timeline->GetPhaseAt(RunTime);
		if (!Phase)
		{
			return; // no phase has started yet
		}
		Target = FMath::Min(Phase->TargetActive, MaxActiveEnemies);
		BatchSize = Phase->BatchSize;
	}

	const int32 ToSpawn = FMath::Clamp(Target - ActiveEnemyCount, 0, BatchSize);

	for (int32 i = 0; i < ToSpawn; ++i)
	{
		const TSubclassOf<AEnemyBase> Class = Phase ? USpawnTimeline::PickEnemy(*Phase) : EnemyClass;
		if (!Class)
		{
			continue;
		}
		FVector GroundLocation;
		if (FindSpawnLocation(PlayerLocation, GetSpawnClearance(Class), GroundLocation))
		{
			SpawnEnemy(Class, GroundLocation, PlayerLocation);
		}
	}
}

AEnemyBase* ASpawnDirector::SpawnEnemy(TSubclassOf<AEnemyBase> Class, const FVector& GroundLocation, const FVector& PlayerLocation)
{
	UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (!Pool || !Class)
	{
		return nullptr;
	}

	AEnemyBase* Enemy = Pool->AcquireAs<AEnemyBase>(Class, GetSpawnTransform(Class, GroundLocation, PlayerLocation));
	if (!Enemy)
	{
		return nullptr;
	}

	// Unique: pooled enemies keep their bindings when reused
	Enemy->OnHit.AddUniqueDynamic(this, &ASpawnDirector::HandleEnemyHit);
	Enemy->OnDied.AddUniqueDynamic(this, &ASpawnDirector::HandleEnemyDied);
	++ActiveEnemyCount;
	ActiveEnemies.Add(Enemy);

	if (Timeline)
	{
		const float Minutes = GetRunTime() / 60.f;
		Enemy->ApplyDifficulty(1.f + Timeline->HealthScalePerMinute * Minutes,
			1.f + Timeline->DamageScalePerMinute * Minutes);
	}
	return Enemy;
}

void ASpawnDirector::ProcessEvents(float RunTime, const FVector& PlayerLocation)
{
	for (int32 i = 0; i < Timeline->Events.Num() && i < NextEventTimes.Num(); ++i)
	{
		if (RunTime < NextEventTimes[i])
		{
			continue;
		}

		const FSpawnEvent& Event = Timeline->Events[i];
		if (!Event.EnemyClass)
		{
			continue;
		}

		// The whole group shares one ring point. If none is found, try again next tick.
		const float Clearance = GetSpawnClearance(Event.EnemyClass);
		FVector Center;
		if (!FindSpawnLocation(PlayerLocation, Clearance, Center))
		{
			continue;
		}

		NextEventTimes[i] = Event.RepeatInterval > 0.f
			? NextEventTimes[i] + Event.RepeatInterval
			: TNumericLimits<float>::Max(); // one-shot: never again

		for (int32 n = 0; n < Event.Count; ++n)
		{
			const FVector2D Offset = FMath::RandPointInCircle(Event.ClusterRadius);
			FVector MemberLocation = Center + FVector(Offset.X, Offset.Y, 0.f);

			// The center is known to be clear; a member whose offset lands in an obstacle starts there instead
			if (!IsSpawnPointClear(MemberLocation, Clearance))
			{
				MemberLocation = Center;
			}
			SpawnEnemy(Event.EnemyClass, MemberLocation, PlayerLocation);
		}
	}
}

float ASpawnDirector::GetRunTime() const
{
	return bRunStarted ? GetWorld()->GetTimeSeconds() - RunStartTime : 0.f;
}

bool ASpawnDirector::FindSpawnLocation(const FVector& PlayerLocation, float ClearanceRadius, FVector& OutLocation) const
{
	FVector2D Corners[4];
	const bool bHaveView = GetVisibleGround(PlayerLocation, Corners);

	constexpr int32 MaxAttempts = 12; // near a wall, about half the ring is outside the map

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		const float Angle = FMath::FRandRange(0.f, UE_TWO_PI);
		const FVector2D Dir(FMath::Cos(Angle), FMath::Sin(Angle));

		// Just past where this direction leaves the screen; a plain circle if the view couldn't be measured
		float Distance = FallbackSpawnRadius;
		if (bHaveView)
		{
			const float Exit = GetExitDistance(FVector2D(PlayerLocation), Dir, Corners);
			if (Exit < MAX_flt)
			{
				Distance = Exit + SpawnMargin;
			}
		}
		const FVector Candidate = PlayerLocation + FVector(Dir, 0.f) * Distance;

		// Trace down for ground or the top of an obstacle; points off the edge of the map hit nothing and get retried
		FCollisionObjectQueryParams GroundOrObstacle(ECC_WorldStatic);
		GroundOrObstacle.AddObjectTypesToQuery(SUCollision::Obstacle);
		FHitResult Hit;
		const FVector TraceStart = Candidate + FVector(0.f, 0.f, 1000.f);
		const FVector TraceEnd = Candidate - FVector(0.f, 0.f, 2000.f);

		if (!GetWorld()->LineTraceSingleByObjectType(Hit, TraceStart, TraceEnd, GroundOrObstacle))
		{
			continue;
		}

		// Landed on top of a pillar, tree, log, boulder, or the jungle wall, or the body would clip into one
		const bool bOnObstacle = Hit.GetComponent() && Hit.GetComponent()->GetCollisionObjectType() == SUCollision::Obstacle;
		if (bOnObstacle || !IsSpawnPointClear(Hit.ImpactPoint, ClearanceRadius))
		{
#if ENABLE_DRAW_DEBUG
			if (SUDebug::IsAIDebugEnabled()) DrawDebugSphere(GetWorld(), Hit.ImpactPoint, ClearanceRadius, 8, FColor::Red, false, 2.f);
#endif
			continue;
		}

#if ENABLE_DRAW_DEBUG
		if (SUDebug::IsAIDebugEnabled()) DrawDebugSphere(GetWorld(), Hit.ImpactPoint, ClearanceRadius, 8, FColor::Green, false, 2.f);
#endif
		OutLocation = Hit.ImpactPoint;
		return true;
	}

	return false;
}

bool ASpawnDirector::IsSpawnPointClear(const FVector& GroundLocation, float ClearanceRadius) const
{
	const FVector Center = GroundLocation + FVector(0.f, 0.f, ClearanceRadius);
	return !GetWorld()->OverlapAnyTestByObjectType(Center, FQuat::Identity,
		FCollisionObjectQueryParams(SUCollision::Obstacle), FCollisionShape::MakeSphere(ClearanceRadius));
}

UCapsuleComponent* ASpawnDirector::GetDefaultCapsule(TSubclassOf<AEnemyBase> Class)
{
	// The class defaults' components are never registered, so measure the capsule's own
		// stored size instead of asking the actor for its collision bounds
	return Class ? Cast<UCapsuleComponent>(Class->GetDefaultObject<AEnemyBase>()->GetRootComponent()) : nullptr;
}

float ASpawnDirector::GetSpawnClearance(TSubclassOf<AEnemyBase> Class) const
{
	const UCapsuleComponent* Capsule = GetDefaultCapsule(Class);
	return (Capsule ? Capsule->GetScaledCapsuleRadius() : 0.f) + ClearancePadding;
}

bool ASpawnDirector::GetVisibleGround(const FVector& PlayerLocation, FVector2D(&OutCorners)[4]) const
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	int32 SizeX = 0, SizeY = 0;
	if (!PC)
	{
		return false;
	}

	PC->GetViewportSize(SizeX, SizeY);
	if (SizeX <= 0 || SizeY <= 0)
	{
		return false;
	}

	// In order around the screen, so each corner and the next one form an edge
	const FVector2D ScreenCorners[4] = {
	FVector2D(0, 0), FVector2D(SizeX, 0), FVector2D(SizeX, SizeY), FVector2D(0, SizeY) };

	for (int32 i = 0; i < 4; ++i)
	{
		FVector WorldPos, WorldDir;
		if (!PC->DeprojectScreenPositionToWorld(ScreenCorners[i].X, ScreenCorners[i].Y, WorldPos, WorldDir)
			|| WorldDir.Z >= -KINDA_SMALL_NUMBER)
		{
			return false; // this corner looks at the sky, so there's no ground shape
		}

		// Where this corner's ray hits the ground plane at the player's height
		const float T = (PlayerLocation.Z - WorldPos.Z) / WorldDir.Z;
		OutCorners[i] = FVector2D(WorldPos + WorldDir * T);
	}
	return true;
}

float ASpawnDirector::GetExitDistance(const FVector2D& Origin, const FVector2D& Dir, const FVector2D(&Corners)[4])
{
	// Solve Origin + t*Dir = A + s*(B - A) for each edge; the exit is the nearest t > 0 with s in [0, 1]
	float Best = MAX_flt;
	for (int32 i = 0; i < 4; ++i)
	{
		const FVector2D A = Corners[i];
		const FVector2D Edge = Corners[(i + 1) % 4] - A;

		const float Denom = FVector2D::CrossProduct(Dir, Edge);
		if (FMath::IsNearlyZero(Denom))
		{
			continue; // parallel to this edge
		}

		const FVector2D ToA = A - Origin;
		const float T = FVector2D::CrossProduct(ToA, Edge) / Denom; // distance along Dir
		const float S = FVector2D::CrossProduct(ToA, Dir) / Denom;  // position along the edge

		if (T > 0.f && S >= 0.f && S <= 1.f)
		{
			Best = FMath::Min(Best, T);
		}
	}
	return Best;
}

void ASpawnDirector::HandleEnemyHit(AEnemyBase* Enemy, float Damage)
{
	OnAnyEnemyHit.Broadcast(Enemy, Damage);
}

void ASpawnDirector::HandleEnemyDied(AEnemyBase* Enemy)
{
	TryDropHealthPickup(Enemy);
	ActiveEnemyCount = FMath::Max(0, ActiveEnemyCount - 1);
	ActiveEnemies.RemoveSingleSwap(Enemy);
	OnAnyEnemyDied.Broadcast(Enemy);
}

void ASpawnDirector::TryDropHealthPickup(const AEnemyBase* Enemy)
{
	const UEnemyData* Data = Enemy ? Enemy->GetEnemyData() : nullptr;
	if (!HealthPickupClass || !Data)
	{
		return;
	}

	const float Chance = CVarForceHealthDrops.GetValueOnGameThread() ? 1.f : Data->HealthDropChance;
	if (FMath::FRand() >= Chance) // did we win the dice roll?
	{
		return; // we lost so don't drop the item
	}

	UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (!Pool)
	{
		return;
	}

	// Off to a random side so it doesn't land on top of the XP gem
	const FVector Offset = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f).Vector() * HealthDropOffset;
	Pool->Acquire(HealthPickupClass, FTransform(FRotator::ZeroRotator, Enemy->GetActorLocation() + Offset));
}

FTransform ASpawnDirector::GetSpawnTransform(TSubclassOf<AEnemyBase> Class, const FVector& GroundLocation, const FVector& PlayerLocation) const
{
	const UCapsuleComponent* Capsule = GetDefaultCapsule(Class);
	const float HalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 0.f;
	const FVector Location = GroundLocation + FVector(0.f, 0.f, HalfHeight + 2.f);
	const FRotator FacePlayer(0.f, (PlayerLocation - Location).Rotation().Yaw, 0.f);
	return FTransform(FacePlayer, Location);
}

void ASpawnDirector::RecycleCheck()
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return;
	}
	const FVector PlayerLocation = Player->GetActorLocation();

	const float Distance = GetRecycleDistance(PlayerLocation);
	const float DistanceSq = Distance * Distance;

	int32 Recycled = 0;
	for (int32 i = ActiveEnemies.Num() - 1; i >= 0 && Recycled < MaxRecyclesPerCheck; --i)
	{
		AEnemyBase* Enemy = ActiveEnemies[i].Get();
		if (!Enemy)
		{
			ActiveEnemies.RemoveAtSwap(i); // destroyed outside the pool (level change)
			continue;
		}
		if (Enemy->IsDead() || FVector::DistSquared2D(Enemy->GetActorLocation(), PlayerLocation) <= DistanceSq)
		{
			continue;
		}

		FVector Ground;
		if (!FindSpawnLocation(PlayerLocation, GetSpawnClearance(Enemy->GetClass()), Ground))
		{
			continue; // no clear spot this time; it gets another chance next check
		}

#if ENABLE_DRAW_DEBUG
		const FVector OldLocation = Enemy->GetActorLocation();
#endif
		// A teleport, not a death: health, statuses, and the active count are all unchanged
		Enemy->SetActorTransform(GetSpawnTransform(Enemy->GetClass(), Ground, PlayerLocation),
			false, nullptr, ETeleportType::TeleportPhysics);
		++Recycled;

#if ENABLE_DRAW_DEBUG
		if (SUDebug::IsAIDebugEnabled()) DrawDebugLine(GetWorld(), OldLocation, Enemy->GetActorLocation(), FColor::Yellow, false, 2.f, 0, 6.f);
#endif
	}
}

int32 ASpawnDirector::SpawnEnemiesNow(TSubclassOf<AEnemyBase> Class, int32 Count)
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player || !Class)
	{
		return 0;
	}

	// Same path as normal spawns: on the ring, clear of obstacles, scaled for the run time.
	// Ignores MaxActiveEnemies on purpose so stress tests can go past it.
	const FVector PlayerLocation = Player->GetActorLocation();
	const float Clearance = GetSpawnClearance(Class);
	int32 Spawned = 0;
	for (int32 i = 0; i < Count; ++i)
	{
		FVector Ground;
		if (FindSpawnLocation(PlayerLocation, Clearance, Ground) && SpawnEnemy(Class, Ground, PlayerLocation))
		{
			++Spawned;
		}
	}
	return Spawned;
}

int32 ASpawnDirector::KillAllEnemies()
{
	// Copy first: each death removes itself from ActiveEnemies
	const TArray<TWeakObjectPtr<AEnemyBase>> ToKill = ActiveEnemies;
	int32 Killed = 0;
	for (const TWeakObjectPtr<AEnemyBase>& Weak : ToKill)
	{
		AEnemyBase* Enemy = Weak.Get();
		if (Enemy && !Enemy->IsDead())
		{
			// Through TakeDamage, so loot drops and the kill counts
			Enemy->TakeDamage(Enemy->GetCurrentHealth() + 1.f, FDamageEvent(UDamageType::StaticClass()), nullptr, nullptr);
			++Killed;
		}
	}
	return Killed;
}

void ASpawnDirector::SkipTime(float Seconds)
{
	if (!bRunStarted || Seconds <= 0.f)
	{
		return;
	}

	RunStartTime -= Seconds;
	const float Now = GetRunTime();

	// Repeating events jump to their next future time instead of firing once per tick to catch up.
		// One-shot events that were skipped (bosses) stay due and fire on the next tick.
	if (Timeline)
	{
		for (int32 i = 0; i < Timeline->Events.Num() && i < NextEventTimes.Num(); ++i)
		{
			const float Interval = Timeline->Events[i].RepeatInterval;
			if (Interval > 0.f && NextEventTimes[i] < Now)
			{
				NextEventTimes[i] += Interval * FMath::CeilToFloat((Now - NextEventTimes[i]) / Interval);
			}
		}
	}
}

void ASpawnDirector::GetSpawnableClasses(TArray<TSubclassOf<AEnemyBase>>& OutClasses) const
{
	OutClasses.Reset();
	if (Timeline)
	{
		TSet<TSubclassOf<AEnemyBase>> Classes;
		Timeline->GetAllEnemyClasses(Classes);
		OutClasses = Classes.Array();
	}
	if (EnemyClass)
	{
		OutClasses.AddUnique(EnemyClass);
	}
}

float ASpawnDirector::GetRecycleDistance(const FVector& PlayerLocation) const
{
	// Never recycle inside the spawn ring, whatever the window size or camera angle
	float MinDistance = FallbackSpawnRadius;
	FVector2D Corners[4];
	if (GetVisibleGround(PlayerLocation, Corners))
	{
		float FarthestSq = 0.f;
		for (const FVector2D& Corner : Corners)
		{
			FarthestSq = FMath::Max(FarthestSq, FVector2D::DistSquared(Corner, FVector2D(PlayerLocation)));
		}
		MinDistance = FMath::Sqrt(FarthestSq) + SpawnMargin;
	}
	return FMath::Max(RecycleDistance, MinDistance + 500.f);
}

void ASpawnDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if ENABLE_DRAW_DEBUG
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player || !SUDebug::IsAIDebugEnabled())
	{
		return;
	}
	const FVector PlayerLocation = Player->GetActorLocation();

	// The visible ground: spawns land just outside this outline
	FVector2D Corners[4];
	if (GetVisibleGround(PlayerLocation, Corners))
	{
		for (int32 i = 0; i < 4; ++i)
		{
			DrawDebugLine(GetWorld(), FVector(Corners[i], PlayerLocation.Z), FVector(Corners[(i + 1) % 4], PlayerLocation.Z),
				FColor::Cyan, false, -1.f, 0, 8.f);
		}
	}

	// Enemies outside this circle get recycled
	DrawDebugCircle(GetWorld(), PlayerLocation, GetRecycleDistance(PlayerLocation), 64, FColor::Magenta, false, -1.f, 0, 6.f,
		FVector(1.f, 0.f, 0.f), FVector(0.f, 1.f, 0.f), false);
#endif
}