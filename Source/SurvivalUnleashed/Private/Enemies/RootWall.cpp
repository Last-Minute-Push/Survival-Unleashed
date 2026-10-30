// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/RootWall.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Framework/ObjectPoolSubsystem.h"
#include "Framework/SUCollision.h"

ARootWall::ARootWall()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->InitBoxExtent(FVector(150.f, 25.f, 150.f));
	Box->SetCollisionProfileName(SUCollision::ObstacleProfile);
	RootComponent = Box;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Box);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetGenerateOverlapEvents(false);
}

void ARootWall::BeginPlay()
{
	Super::BeginPlay();

	// Pooled actors run BeginPlay once, before any rise moves the mesh
	MeshRestZ = Mesh->GetRelativeLocation().Z;
}

void ARootWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	UpdateMeshHeight();
}

void ARootWall::OnPoolAcquire_Implementation()
{
	Age = 0.f;
	UpdateMeshHeight();
	SetActorTickEnabled(true);
	GetWorldTimerManager().SetTimer(LifetimeTimer, this, &ARootWall::Expire, Lifetime, false);
}

void ARootWall::OnPoolRelease_Implementation()
{
	GetWorldTimerManager().ClearTimer(LifetimeTimer);
	SetActorTickEnabled(false);
}

void ARootWall::Expire()
{
	if (UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
	{
		Pool->Release(this);
	}
}

void ARootWall::PushOutEnemies() const
{
	const FVector Extent = Box->GetScaledBoxExtent();
	const FVector Center = Box->GetComponentLocation();
	const FVector Across = GetActorRightVector(); // the thickness axis

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, Center, Box->GetComponentQuat(),
		FCollisionObjectQueryParams(SUCollision::Enemy), FCollisionShape::MakeBox(Extent));

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Enemy = Overlap.GetActor();
		if (!Enemy)
		{
			continue;
		}

		const UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(Enemy->GetRootComponent());
		const float Radius = Capsule ? Capsule->GetScaledCapsuleRadius() : 50.f;

		// Out to whichever face is closer, plus the enemy's radius so it isn't left touching the wall
		const float Offset = FVector::DotProduct(Enemy->GetActorLocation() - Center, Across);
		const float Side = Offset >= 0.f ? 1.f : -1.f;
		const float Push = Side * (Extent.Y + Radius + 5.f) - Offset;

		Enemy->SetActorLocation(Enemy->GetActorLocation() + Across * Push, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

float ARootWall::GetHalfHeight() const
{
	// Class default components are never registered, so read the stored extent, not the world bounds
	return Box ? Box->GetScaledBoxExtent().Z : 0.f;
}

void ARootWall::UpdateMeshHeight()
{
	// 0 = fully underground, 1 = at rest. Bursts up fast and settles, sinks back at the end.
	const float Rise = FMath::InterpEaseOut(0.f, 1.f, FMath::Clamp(Age / RiseTime, 0.f, 1.f), 2.f);
	const float Sink = FMath::Clamp((Lifetime - Age) / SinkTime, 0.f, 1.f);
	const float Up = FMath::Min(Rise, Sink);

	const float FullHeight = Box->GetUnscaledBoxExtent().Z * 2.f;
	FVector Location = Mesh->GetRelativeLocation();
	Location.Z = MeshRestZ - (1.f - Up) * FullHeight;
	Mesh->SetRelativeLocation(Location);
}