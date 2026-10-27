#include "Enemies/Hornet.h"
#include "Enemies/HordeSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

AHornet::AHornet()
{
	Capsule->InitCapsuleSize(20.f, 20.f);
	SeparationRadius = 50.f;
}

void AHornet::OnPoolAcquire_Implementation()
{
	Super::OnPoolAcquire_Implementation();
	NoiseSeed = FMath::FRandRange(0.f, 1000.f);
}

FVector AHornet::GetDesiredDirection() const
{
	const FVector MyLocation = GetActorLocation();

	// Seek
	FVector Seek = FVector::ZeroVector;
	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		const FVector ToPlayer = Player->GetActorLocation() - MyLocation;
		if (ToPlayer.SizeSquared2D() > FMath::Square(StopDistance))
		{
			Seek = ToPlayer.GetSafeNormal2D();
		}
	}

	// One neighbor query feeds all three flocking forces
	FVector Separation = FVector::ZeroVector;
	FVector Alignment = FVector::ZeroVector;
	FVector CenterSum = FVector::ZeroVector;
	int32 FlockCount = 0;

	if (UHordeSubsystem* HordeSystem = GetHorde())
	{
		HordeSystem->GetNeighbors(MyLocation, NeighborRadius, FlockScratch);

		for (const AEnemyBase* Other : FlockScratch)
		{
			if (Other == this)
			{
				continue;
			}

			FVector Away = MyLocation - Other->GetActorLocation();
			Away.Z = 0.f;
			const float Dist = Away.Size();

			// Separation: from every enemy that's too close
			if (Dist > KINDA_SMALL_NUMBER && Dist < SeparationRadius)
			{
				Separation += (Away / Dist) * (1.f - Dist / SeparationRadius);
			}

			// Alignment and cohesion: only with other hornets
			if (Dist < NeighborRadius && Other->IsA<AHornet>())
			{
				Alignment += Other->GetVelocity().GetSafeNormal2D();
				CenterSum += Other->GetActorLocation();
				++FlockCount;
			}
		}
	}

	FVector Cohesion = FVector::ZeroVector;
	if (FlockCount > 0)
	{
		Alignment = (Alignment / FlockCount).GetClampedToMaxSize2D(1.f);

		FVector ToCenter = CenterSum / FlockCount - MyLocation;
		ToCenter.Z = 0.f;
		// Stronger the farther this hornet has drifted from the group
		Cohesion = (ToCenter / NeighborRadius).GetClampedToMaxSize2D(1.f);
	}

	// Wander: a smoothly changing direction, unique per hornet
	const float Time = GetWorld()->GetTimeSeconds();
	const float WanderAngle = FMath::PerlinNoise1D(Time * WanderFrequency + NoiseSeed) * UE_TWO_PI;
	const FVector Wander(FMath::Cos(WanderAngle), FMath::Sin(WanderAngle), 0.f);

	FVector Desired = Seek * SeekWeight
		+ Separation.GetClampedToMaxSize2D(1.f) * SeparationWeight
		+ Alignment * AlignmentWeight
		+ Cohesion * CohesionWeight
		+ Wander * WanderWeight;
	Desired = Desired.GetClampedToMaxSize2D(1.f);

	// Dart: the base Tick multiplies this by MoveSpeed, so a length above 1 means faster
	const float SpeedFactor = 1.f + SpeedVariation * FMath::PerlinNoise1D(Time * 2.f + NoiseSeed * 3.f);
	return Desired * SpeedFactor;
}