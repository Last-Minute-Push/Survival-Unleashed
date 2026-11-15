// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/RootWorm.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ARootWorm::ARootWorm()
{
	// Absolute location and rotation: the ring is placed in the world and stays there while the worm moves to it
	RingDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("RingDecal"));
	RingDecal->SetupAttachment(GetRootComponent());
	RingDecal->SetUsingAbsoluteLocation(true);
	RingDecal->SetUsingAbsoluteRotation(true);
	RingDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f)); // project straight down
	RingDecal->SetFadeScreenSize(0.f);                         // never fade out with distance
	RingDecal->SetVisibility(false);

	// Purely visual
	RippleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RippleMesh"));
	RippleMesh->SetupAttachment(GetRootComponent());
	RippleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RippleMesh->SetGenerateOverlapEvents(false);
	RippleMesh->SetCastShadow(false);
}

void ARootWorm::BeginPlay()
{
	Super::BeginPlay();

	// The decal material is set in the Blueprint; this copy lets each worm fill its own ring
	RingMaterial = RingDecal->CreateDynamicMaterialInstance();

	// X is the projection depth; Y and Z are the half-size of the square the ring is drawn in
	RingDecal->DecalSize = FVector(64.f, AmbushRadius, AmbushRadius);
	RingDecal->MarkRenderStateDirty();

	// The actor's origin is the capsule's center, so the ground is one half height down
	RippleMesh->SetRelativeLocation(FVector(0.f, 0.f, -GetHalfHeight()));
}

void ARootWorm::OnPoolAcquire_Implementation()
{
	Super::OnPoolAcquire_Implementation();

	State = EWormState::Tunneling;
	StateElapsed = 0.f;
	CooldownRemaining = FirstAmbushDelay;

	SetRingVisible(false);
	RippleMesh->SetVisibility(true);
	SetMaterialData(EnemyMaterialData::AttackPose, 0.f);
}

FVector ARootWorm::GetDesiredDirection() const
{
	// Chases only while tunneling; the warning dash and the time above ground are handled here
	return State == EWormState::Tunneling ? Super::GetDesiredDirection() : FVector::ZeroVector;
}

bool ARootWorm::IsTargetable() const
{
	return State == EWormState::Surfaced && Super::IsTargetable();
}

float ARootWorm::GetContactDamage() const
{
	return State == EWormState::Surfaced ? Super::GetContactDamage() : 0.f;
}

float ARootWorm::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	// Underground: area damage, burn ticks and anything else that doesn't check IsTargetable does nothing
	// (skipping Super also skips the hit flash, so nothing looks like it landed)
	if (State != EWormState::Surfaced)
	{
		return 0.f;
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ARootWorm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead())
	{
		return;
	}

	// Frozen or stunned: every timer below waits
	const float Step = CanAdvanceTimers() ? DeltaTime : 0.f;

	switch (State)
	{
	case EWormState::Tunneling:
	{
		CooldownRemaining = FMath::Max(0.f, CooldownRemaining - Step);
		if (CooldownRemaining > 0.f || Step == 0.f)
		{
			break;
		}

		const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
		if (Player && FVector::DistSquared2D(GetActorLocation(), Player->GetActorLocation()) <= FMath::Square(AmbushRange))
		{
			StartWarning(*Player);
		}
		break;
	}

	case EWormState::Warning:
	{
		StateElapsed += Step;
		const float Alpha = FMath::Clamp(StateElapsed / WarningTime, 0.f, 1.f);

		// Races to the spot underground, arriving as the ring fills. The ripple shows it coming.
		// No sweep: it's underground, so obstacles don't stop it.
		SetActorLocation(FMath::Lerp(WarningStart, AmbushLocation, FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f)));
		SetRingVisible(true, Alpha);

		if (Alpha >= 1.f)
		{
			Erupt();
		}
		break;
	}

	case EWormState::Surfaced:
	{
		StateElapsed += Step;

		// Pops out fast, holds, then sinks back over the last BurrowTime
		const float Out = FMath::Clamp(StateElapsed / EmergeTime, 0.f, 1.f);
		const float In = FMath::Clamp((SurfaceTime - StateElapsed) / BurrowTime, 0.f, 1.f);
		SetMaterialData(EnemyMaterialData::AttackPose, FMath::Min(Out, In));

		if (StateElapsed >= SurfaceTime)
		{
			Burrow();
		}
		break;
	}
	}
}

bool ARootWorm::CanAdvanceTimers() const
{
	return !IsStatusActive(EEnemyStatus::Freeze) && !IsStatusActive(EEnemyStatus::Stun);
}

void ARootWorm::StartWarning(const APawn& Player)
{
	State = EWormState::Warning;
	StateElapsed = 0.f;

	// Locks where the player is now, kept at the worm's own height so it comes up level with the ground
	WarningStart = GetActorLocation();
	const FVector PlayerLocation = Player.GetActorLocation();
	AmbushLocation = FVector(PlayerLocation.X, PlayerLocation.Y, WarningStart.Z);

	RingDecal->SetWorldLocation(AmbushLocation - FVector(0.f, 0.f, GetHalfHeight()));
	SetRingVisible(true, 0.f);
}

void ARootWorm::Erupt()
{
	State = EWormState::Surfaced;
	StateElapsed = 0.f;
	SetRingVisible(false);
	RippleMesh->SetVisibility(false);

	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return;
	}

	// Same flat-circle check as the poison cloud: a player who left the ring takes nothing
	const FVector ToPlayer = Player->GetActorLocation() - AmbushLocation;
	if (ToPlayer.SizeSquared2D() > FMath::Square(AmbushRadius) || FMath::Abs(ToPlayer.Z) > 200.f)
	{
		return;
	}

	// Same path as contact damage, so Defense, Evasion and dash invincibility apply
	const float Dealt = UGameplayStatics::ApplyDamage(Player, EruptionDamage, nullptr, this, nullptr);

	// A dodged hit (the player's TakeDamage returned 0) doesn't knock them back either
	ACharacter* PlayerCharacter = Cast<ACharacter>(Player);
	if (Dealt > 0.f && PlayerCharacter)
	{
		// Straight out from the center; a player dead center gets pushed the way the worm was heading
		FVector Away = ToPlayer.GetSafeNormal2D();
		if (Away.IsNearlyZero())
		{
			Away = (AmbushLocation - WarningStart).GetSafeNormal2D();
		}

		// A small hop: friction stops a grounded launch almost at once, so the push only carries while airborne
		PlayerCharacter->LaunchCharacter(Away * KnockbackSpeed + FVector(0.f, 0.f, KnockbackSpeed * 0.3f), true, true);
	}
}

void ARootWorm::Burrow()
{
	State = EWormState::Tunneling;
	StateElapsed = 0.f;
	CooldownRemaining = AmbushCooldown;

	RippleMesh->SetVisibility(true);
	SetMaterialData(EnemyMaterialData::AttackPose, 0.f);
}

float ARootWorm::GetHalfHeight() const
{
	const UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(GetRootComponent());
	return CapsuleComp ? CapsuleComp->GetScaledCapsuleHalfHeight() : 0.f;
}

void ARootWorm::SetRingVisible(bool bVisible, float Progress)
{
	RingDecal->SetVisibility(bVisible);
	if (bVisible && RingMaterial)
	{
		RingMaterial->SetScalarParameterValue(RingProgressParam, Progress);
	}
}
