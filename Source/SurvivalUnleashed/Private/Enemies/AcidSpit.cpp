// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/AcidSpit.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Pawn.h"
#include "Framework/SUCollision.h"

AAcidSpit::AAcidSpit()
{
	// Start from the player-attack responses (overlaps obstacles, so it stops on them) and flip who it touches
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(SUCollision::Enemy, ECR_Ignore);

	// The player's pickup sphere is 300 uu wide; without this the acid would "hit" from that far away
	Collision->SetCollisionResponseToChannel(SUCollision::PickupRadius, ECR_Ignore);

	// A handful of plants at most, spitting every 2.5 s
	PrewarmCount = 20;
}

bool AAcidSpit::IsValidTarget(const AActor* Target) const
{
	const APawn* Pawn = Cast<APawn>(Target);
	return Pawn && Pawn->IsPlayerControlled();
}
