// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/BossChest.h"
#include "Actor/UC_Player.h"
#include "Utility/PlayerStatsComponent.h"
#include "Framework/ObjectPoolSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

ABossChest::ABossChest()
{
	// Nothing to update: it waits for the player
	PrimaryActorTick.bCanEverTick = false;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->InitSphereRadius(120.f);
	PickupSphere->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	PickupSphere->SetGenerateOverlapEvents(true);
	RootComponent = PickupSphere;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(PickupSphere);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetGenerateOverlapEvents(false);

	Beam = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Beam"));
	Beam->SetupAttachment(PickupSphere);
	Beam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Beam->SetGenerateOverlapEvents(false);
	Beam->SetCastShadow(false);

	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &ABossChest::HandleOverlap);
}

void ABossChest::OnPoolAcquire_Implementation()
{
	// Pooled chests are reused
	bCollected = false;
}

void ABossChest::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bCollected)
	{
		return;
	}

	// Only the player's body counts. The player's pickup-range sphere overlaps the chest too,
	// and it grows with Pickup Range upgrades, so it would collect the chest from far away.
	AUC_Player* Player = Cast<AUC_Player>(OtherActor);
	if (!Player || OtherComp != Player->GetCapsuleComponent())
	{
		return;
	}

	UPlayerStatsComponent* Stats = Player->GetPlayerStatsComp();
	if (!Stats)
	{
		return;
	}

	bCollected = true;

	// The menus open from the level-up queue, after any level-ups already waiting, so nothing opens twice
	Stats->QueueBossChest();

	if (UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
	{
		Pool->Release(this);
	}
	else
	{
		Destroy();
	}
}
