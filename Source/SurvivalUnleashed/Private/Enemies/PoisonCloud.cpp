// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/PoisonCloud.h"
#include "Components/StaticMeshComponent.h"
#include "Framework/ObjectPoolSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

APoisonCloud::APoisonCloud()
{
	PrimaryActorTick.bCanEverTick = true; // the pool turns ticking on while the cloud is out

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Purely visual: damage is a distance check, so the cloud never blocks or overlaps anything
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetCastShadow(false);
}

void APoisonCloud::BeginPlay()
{
	Super::BeginPlay();

	// Pooled actors run BeginPlay once, before any puff changes the scale
	MeshRestScale = Mesh->GetRelativeScale3D();
}

void APoisonCloud::OnPoolAcquire_Implementation()
{
	Age = 0.f;
	UpdateLook();

	// First damage one interval after the burst: a reaction window, then steady ticks
	GetWorldTimerManager().SetTimer(DamageTimer, this, &APoisonCloud::DamageTick, TickInterval, true, TickInterval);
	GetWorldTimerManager().SetTimer(LifetimeTimer, this, &APoisonCloud::Expire, Lifetime, false);
}

void APoisonCloud::OnPoolRelease_Implementation()
{
	GetWorldTimerManager().ClearTimer(DamageTimer);
	GetWorldTimerManager().ClearTimer(LifetimeTimer);
}

void APoisonCloud::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Age += DeltaTime;
	UpdateLook();
}

void APoisonCloud::DamageTick()
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return;
	}

	// Flat circle on the ground; the height check just keeps it from reaching a player far above or below
	const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	if (ToPlayer.SizeSquared2D() <= FMath::Square(Radius) && FMath::Abs(ToPlayer.Z) <= 300.f)
	{
		// Same path as contact damage, so Defense, Evasion and dash invincibility apply
		UGameplayStatics::ApplyDamage(Player, DamagePerTick, nullptr, this, nullptr);
	}
}

void APoisonCloud::Expire()
{
	if (UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
	{
		Pool->Release(this);
	}
}

void APoisonCloud::UpdateLook()
{
	// Puffs out from 60% size, holds, then thins out (the material uses CPD 0 as opacity)
	const float PuffAlpha = FMath::Clamp(Age / PuffTime, 0.f, 1.f);
	const float FadeAlpha = FMath::Clamp((Lifetime - Age) / FadeTime, 0.f, 1.f);

	Mesh->SetRelativeScale3D(MeshRestScale * FMath::Lerp(0.6f, 1.f, FMath::InterpEaseOut(0.f, 1.f, PuffAlpha, 2.f)));
	Mesh->SetCustomPrimitiveDataFloat(0, FMath::Min(PuffAlpha, FadeAlpha));
}
