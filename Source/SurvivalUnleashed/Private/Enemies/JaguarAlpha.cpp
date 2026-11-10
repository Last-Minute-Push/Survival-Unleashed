// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/JaguarAlpha.h"
#include "Enemies/JungleCat.h"
#include "Enemies/SpawnDirector.h"
#include "Framework/ObjectPoolSubsystem.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

namespace
{
	const FName PounceProgressParam(TEXT("Progress"));
}

AJaguarAlpha::AJaguarAlpha()
{
	// Absolute location and rotation: the ring is placed on the landing spot and stays there while the Jaguar moves
	PounceDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("PounceDecal"));
	PounceDecal->SetupAttachment(GetRootComponent());
	PounceDecal->SetUsingAbsoluteLocation(true);
	PounceDecal->SetUsingAbsoluteRotation(true);
	PounceDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f)); // project straight down
	PounceDecal->SetFadeScreenSize(0.f);
	PounceDecal->SetVisibility(false);
}

void AJaguarAlpha::BeginPlay()
{
	Super::BeginPlay();

	// Sized here so a Blueprint's PounceRadius is used. X is the projection depth.
	PounceDecal->DecalSize = FVector(150.f, PounceRadius, PounceRadius);
	PounceDecal->MarkRenderStateDirty();
	PounceDecalMaterial = PounceDecal->CreateDynamicMaterialInstance();

	// Every pooled Jaguar runs this, so only the first one fills the cat pool
	UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (Pool && CatClass && Pool->GetInactiveCount(CatClass) == 0)
	{
		Pool->Prewarm(CatClass, CatPrewarm);
	}
}

void AJaguarAlpha::OnFightStarted()
{
	// Each attack comes after one full interval, so the fight opens with the Jaguar circling in
	State = EJaguarState::Stalk;
	StateElapsed = 0.f;
	PounceCooldown = PounceInterval;
	PackCooldown = PackInterval;
	OrbitSign = FMath::RandBool() ? 1.f : -1.f;
	ResetFlipTimer();
	Pack.Reset();

	SetPounceTelegraph(false);
	SetMaterialData(EnemyMaterialData::AttackPose, 0.f);
}

void AJaguarAlpha::OnFightEnded()
{
	SetPounceTelegraph(false);
	ReleasePack();
}

float AJaguarAlpha::GetContactDamage() const
{
	return State == EJaguarState::Leap ? 0.f : Super::GetContactDamage();
}

void AJaguarAlpha::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsFightActive() || IsDead())
	{
		return;
	}

	// Committed: a leap in the air finishes even if it's frozen or stunned on the way
	if (State == EJaguarState::Leap)
	{
		UpdateLeap(DeltaTime);
		return;
	}

	// Frozen or stunned: everything else waits
	if (!CanAdvanceTimers())
	{
		return;
	}

	PounceCooldown -= DeltaTime;
	PackCooldown -= DeltaTime;
	StateElapsed += DeltaTime;

	switch (State)
	{
	case EJaguarState::Crouch:
	{
		const float Alpha = FMath::Clamp(StateElapsed / PounceCrouchTime, 0.f, 1.f);
		SetMaterialData(EnemyMaterialData::AttackPose, 0.5f * FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.f));
		SetPounceTelegraph(true, Alpha);
		TurnTowards(LeapEnd, DeltaTime);

		if (Alpha >= 1.f)
		{
			StartLeap();
		}
		break;
	}

	case EJaguarState::Recover:
	{
		// From the landing pose (-1) back to standing (0)
		const float Alpha = FMath::Clamp(StateElapsed / PounceRecoverTime, 0.f, 1.f);
		SetMaterialData(EnemyMaterialData::AttackPose, -1.f + FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.f));

		if (Alpha >= 1.f)
		{
			State = EJaguarState::Stalk;
			StateElapsed = 0.f;
			SetMaterialData(EnemyMaterialData::AttackPose, 0.f);
		}
		break;
	}

	case EJaguarState::Stalk:
	{
		FlipCooldown -= DeltaTime;
		if (FlipCooldown <= 0.f)
		{
			OrbitSign = -OrbitSign;
			ResetFlipTimer();
		}

		// At most one attack per frame. The pack first: it's the rarer one.
		if (PackCooldown <= 0.f)
		{
			CallThePack();
			PackCooldown = PackInterval;
		}
		else if (PounceCooldown <= 0.f)
		{
			// Out of range: try again next frame
			TryStartPounce();
		}
		break;
	}

	default:
		break;
	}
}

FVector AJaguarAlpha::GetDesiredDirection() const
{
	// Only moves on its own while stalking; the leap is moved by UpdateLeap
	if (State != EJaguarState::Stalk)
	{
		return FVector::ZeroVector;
	}

	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return FVector::ZeroVector;
	}

	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	const float Distance = ToPlayer.Size();
	if (Distance < KINDA_SMALL_NUMBER)
	{
		return Super::GetDesiredDirection();
	}

	// Around the circle, steering in or out by how far off it we are:
	// right on the circle it runs sideways, StalkCorrection or more off it heads straight in or out
	const FVector In = ToPlayer / Distance;
	const FVector Around = FVector::CrossProduct(FVector::UpVector, In) * OrbitSign;
	const float Off = FMath::Clamp((Distance - StalkRadius) / StalkCorrection, -1.f, 1.f);

	const FVector Desired = (Around * (1.f - FMath::Abs(Off)) + In * Off).GetSafeNormal2D();
	return (Desired * SeekWeight + ComputeSeparation() * SeparationWeight).GetClampedToMaxSize2D(1.f);
}

// ---------------------------------------------------------------- Pounce

bool AJaguarAlpha::TryStartPounce()
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return false;
	}

	const FVector Start = GetActorLocation();
	const FVector Target = Player->GetActorLocation();
	const float Distance = FVector::Dist2D(Start, Target);
	if (Distance < PounceMinRange || Distance > PounceMaxRange)
	{
		return false;
	}

	// Locked now: the ring shows exactly where it will land, and the player has the crouch to get out
	if (!FindLandingSpot(Start, FVector(Target.X, Target.Y, Start.Z), LeapEnd))
	{
		PounceCooldown = 1.f; // nowhere clear to land; look again shortly
		return false;
	}

	State = EJaguarState::Crouch;
	StateElapsed = 0.f;

	PounceDecal->SetWorldLocation(LeapEnd - FVector(0.f, 0.f, Capsule->GetScaledCapsuleHalfHeight()));
	SetPounceTelegraph(true, 0.f);
	return true;
}

void AJaguarAlpha::StartLeap()
{
	State = EJaguarState::Leap;
	StateElapsed = 0.f;
	LeapStart = GetActorLocation();
	SetPounceTelegraph(true, 1.f); // the ring stays up, full, until the landing
}

void AJaguarAlpha::UpdateLeap(float DeltaTime)
{
	StateElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(StateElapsed / PounceLeapTime, 0.f, 1.f);

	// Along the ground, no sweep: it's jumping over whatever is in the way. The material draws the arc.
	SetActorLocation(FMath::Lerp(LeapStart, LeapEnd, Alpha));
	SetMaterialData(EnemyMaterialData::AttackPose, 0.5f + 0.5f * Alpha);

	const FVector Direction = (LeapEnd - LeapStart).GetSafeNormal2D();
	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(FRotator(0.f, Direction.Rotation().Yaw, 0.f));
	}

	if (Alpha >= 1.f)
	{
		Land();
	}
}

void AJaguarAlpha::Land()
{
	SetActorLocation(LeapEnd);
	SetPounceTelegraph(false);

	State = EJaguarState::Recover;
	StateElapsed = 0.f;
	PounceCooldown = PounceInterval;
	SetMaterialData(EnemyMaterialData::AttackPose, -1.f);

	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return;
	}

	// Same flat-circle check as the other ground attacks: a player who left the ring takes nothing
	const FVector ToPlayer = Player->GetActorLocation() - LeapEnd;
	if (ToPlayer.SizeSquared2D() <= FMath::Square(PounceRadius) && FMath::Abs(ToPlayer.Z) <= 200.f)
	{
		// Same path as contact damage, so Defense, Evasion and Thorns apply
		UGameplayStatics::ApplyDamage(Player, PounceDamage, nullptr, this, nullptr);
	}
}

bool AJaguarAlpha::FindLandingSpot(const FVector& Start, const FVector& Target, FVector& OutCenter) const
{
	const float Radius = Capsule->GetScaledCapsuleRadius();
	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	FVector Path = Target - Start;
	Path.Z = 0.f;
	const float Length = Path.Size();
	const FVector Direction = Length > KINDA_SMALL_NUMBER ? Path / Length : GetActorForwardVector();

	// The target first, then one capsule radius at a time back toward the Jaguar
	const float Step = FMath::Max(Radius, 25.f);
	for (float Back = 0.f; Back <= Length; Back += Step)
	{
		FVector Ground;
		if (FindGround(Start + Direction * (Length - Back), true, Ground, Radius + 10.f))
		{
			OutCenter = Ground + FVector(0.f, 0.f, HalfHeight);
			return true;
		}
	}
	return false;
}

void AJaguarAlpha::SetPounceTelegraph(bool bVisible, float Progress)
{
	PounceDecal->SetVisibility(bVisible);
	if (PounceDecalMaterial)
	{
		PounceDecalMaterial->SetScalarParameterValue(PounceProgressParam, FMath::Clamp(Progress, 0.f, 1.f));
	}
}

// ---------------------------------------------------------------- Call the Pack

void AJaguarAlpha::CallThePack()
{
	ASpawnDirector* Director = GetSpawnDirector();
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Director || !Player || !CatClass)
	{
		return;
	}

	const FVector PlayerLocation = Player->GetActorLocation();
	const float LungeTime = GetWorld()->GetTimeSeconds() + PackGatherTime + PackCircleTime;

	// Places on the circle are spread evenly, starting on the Jaguar's side, so the nearest cats go to the nearest places
	const FVector FromPlayer = GetActorLocation() - PlayerLocation;
	const float BaseAngle = FMath::Atan2(FromPlayer.Y, FromPlayer.X);

	Pack.Reset(); // the last pack lunged long ago (PackInterval is much longer than the gather and circle)
	for (int32 i = 0; i < PackSize; ++i)
	{
		const float Angle = BaseAngle + UE_TWO_PI * i / PackSize;
		const FVector Point = GetActorLocation() + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * PackSpawnRadius;

		FVector Ground;
		if (!FindGround(Point, true, Ground))
		{
			continue;
		}

		// Through the director, so cats count as active enemies, scale with time and drop loot
		if (AJungleCat* Cat = Cast<AJungleCat>(Director->SpawnEnemy(CatClass, Ground, PlayerLocation)))
		{
			Cat->JoinPack(Angle, LungeTime);
			Pack.Add(Cat);
		}
	}
}

void AJaguarAlpha::ReleasePack()
{
	for (const TWeakObjectPtr<AJungleCat>& Weak : Pack)
	{
		AJungleCat* Cat = Weak.Get();
		if (Cat && !Cat->IsDead())
		{
			Cat->LeavePack();
		}
	}
	Pack.Reset();
}

// ---------------------------------------------------------------- Helpers

bool AJaguarAlpha::CanAdvanceTimers() const
{
	return !IsStatusActive(EEnemyStatus::Freeze) && !IsStatusActive(EEnemyStatus::Stun);
}

void AJaguarAlpha::TurnTowards(const FVector& Point, float DeltaTime)
{
	const FVector Direction = (Point - GetActorLocation()).GetSafeNormal2D();
	if (!Direction.IsNearlyZero())
	{
		const FRotator Target(0.f, Direction.Rotation().Yaw, 0.f);
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), Target, DeltaTime, TurnSpeed));
	}
}

void AJaguarAlpha::ResetFlipTimer()
{
	FlipCooldown = FMath::FRandRange(StalkFlipMin, FMath::Max(StalkFlipMin, StalkFlipMax));
}
