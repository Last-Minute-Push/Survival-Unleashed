// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/SporePuffball.h"
#include "Enemies/PoisonCloud.h"
#include "Framework/ObjectPoolSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void ASporePuffball::BeginPlay()
{
	Super::BeginPlay();

	// Every pooled puffball runs this, so only the first one fills the cloud pool
	UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (Pool && CloudClass && Pool->GetInactiveCount(CloudClass) == 0)
	{
		Pool->Prewarm(CloudClass, CloudPrewarm);
	}
}

void ASporePuffball::OnPoolAcquire_Implementation()
{
	Super::OnPoolAcquire_Implementation();
	bSwelling = false;
	SwellElapsed = 0.f;
	SetMaterialData(EnemyMaterialData::AttackPose, 0.f);
}

FVector ASporePuffball::GetDesiredDirection() const
{
	// Plants itself to swell
	return bSwelling ? FVector::ZeroVector : Super::GetDesiredDirection();
}

void ASporePuffball::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead())
	{
		return;
	}

	if (!bSwelling)
	{
		const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
		if (Player && FVector::DistSquared2D(GetActorLocation(), Player->GetActorLocation()) <= FMath::Square(SwellTriggerRange))
		{
			bSwelling = true;
			SwellElapsed = 0.f;
		}
		return;
	}

	// Frozen or stunned: the swell waits (the cloud still comes, just later)
	if (!IsStatusActive(EEnemyStatus::Freeze) && !IsStatusActive(EEnemyStatus::Stun))
	{
		SwellElapsed += DeltaTime;
	}

	SetMaterialData(EnemyMaterialData::AttackPose, FMath::Clamp(SwellElapsed / SwellTime, 0.f, 1.f));

	if (SwellElapsed >= SwellTime)
	{
		Burst();
	}
}

float ASporePuffball::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	// Read before the hit: dying releases us to the pool. A hit on an already-dead puffball doesn't count.
	const bool bWasAlive = !IsDead();
	const FVector Location = GetActorLocation();

	const float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Killed by the player (or a burn tick): the cloud still comes out, and the gem drops as normal
	if (bWasAlive && IsDead())
	{
		SpawnCloud(Location);
	}
	return Damage;
}

void ASporePuffball::Burst()
{
	SpawnCloud(GetActorLocation());

	// Its own burst: dies (OnDied still fires, so the Spawn Director's count stays right) but drops no gem
	Die(/*bDropLoot*/ false);
}

void ASporePuffball::SpawnCloud(const FVector& ActorLocation) const
{
	UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (!Pool || !CloudClass)
	{
		return;
	}

	const UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(GetRootComponent());
	const float HalfHeight = CapsuleComp ? CapsuleComp->GetScaledCapsuleHalfHeight() : 0.f;
	Pool->Acquire(CloudClass, FTransform(ActorLocation - FVector(0.f, 0.f, HalfHeight)));
}
