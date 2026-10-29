#pragma once

#include "Engine/EngineTypes.h"

// Must match [/Script/Engine.CollisionProfile] in DefaultEngine.ini
namespace SUCollision
{
	constexpr ECollisionChannel Enemy = ECC_GameTraceChannel1;
	constexpr ECollisionChannel PickupRadius = ECC_GameTraceChannel2;
	constexpr ECollisionChannel Obstacle = ECC_GameTraceChannel3;

	inline const FName ObstacleProfile = TEXT("Obstacle");
}