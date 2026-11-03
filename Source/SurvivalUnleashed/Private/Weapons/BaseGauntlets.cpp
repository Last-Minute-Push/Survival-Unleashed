// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/BaseGauntlets.h"
#include "Components/BoxComponent.h"
#include "Weapons/WeaponData.h"

DEFINE_LOG_CATEGORY_STATIC(LogBaseGauntlets, Log, All);

ABaseGauntlets::ABaseGauntlets() : Left(StateCooldown), Right(StateCooldown)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Component Initialization
	Left.GauntletPivot  = GauntletPivotLeft  = CreateDefaultSubobject<USceneComponent>("GauntletPivotLeft");
	Left.Mesh           = MeshLeft           = CreateDefaultSubobject<USkeletalMeshComponent>("MeshLeft");
	Left.BoxCollision   = BoxCollisionLeft   = CreateDefaultSubobject<UBoxComponent>("BoxCollisionLeft");

	Right.GauntletPivot = GauntletPivotRight = CreateDefaultSubobject<USceneComponent>("GauntletPivotRight");
	Right.Mesh          = MeshRight          = CreateDefaultSubobject<USkeletalMeshComponent>("MeshRight");
	Right.BoxCollision  = BoxCollisionRight  = CreateDefaultSubobject<UBoxComponent>("BoxCollisionRight");

	// Component Attachment
	GauntletPivotLeft->SetupAttachment(Pivot);
	MeshLeft->SetupAttachment(GauntletPivotLeft);
	BoxCollisionLeft->SetupAttachment(MeshLeft);

	GauntletPivotRight->SetupAttachment(Pivot);
	MeshRight->SetupAttachment(GauntletPivotRight);
	BoxCollisionRight->SetupAttachment(MeshRight);

	// State Machine
	StatePunch = FGauntletState(Punch, MoveGauntlets, EndPunch);
	StateRetreat  = FGauntletState(Retreat, MoveGauntlets, EndRetreat);

	// Final Gauntlet initializatons
	Left.Other = &Right;
	Right.Other = &Left;

	First = &Right;
}

void ABaseGauntlets::BeginPlay()
{
	Super::BeginPlay();

	// - Gauntlets Properties - 
	if (Data)
	{
		Range          = Data->Range / FMath::Max(ArcRadius + 1.f, 1.f);
		Height         = Data->Height;
		Radius         = Data->DistanceFromPlayer;
		Cooldown       = Data->Cooldown;
		Left.Direction = -(Right.Direction = Data->AttackSpeed / FMath::Max(ArcRadius + 1.f, 1.f));
	}
	else
	{
		UE_LOG(LogBaseGauntlets, Error, TEXT("ABaseGauntlets' WeaponData asset was not set!"));

		Range    = 0.f;
		Height   = 0.f;
		Radius   = 0.f;
		Cooldown = 0.f;
		Left.Direction = Right.Direction = 0.f;
	}

	// - Location -
	
	// Radius and Width (DistanceBetweenGauntlets)
	Left.GauntletPivot->AddLocalOffset(FVector(-DistanceBetweenGauntlets + ArcRadius, Radius, 0.f));
	Right.GauntletPivot->AddLocalOffset(FVector(DistanceBetweenGauntlets - ArcRadius, Radius, 0.f));
	// Arc Radius
	Left.Mesh->AddLocalOffset(FVector(-ArcRadius, 0.f, 0.f));
	Right.Mesh->AddLocalOffset(FVector(ArcRadius, 0.f, 0.f));

	GetWorldTimerManager().SetTimer(TPunchCooldown, [this]()
	{
		if (!First.IsValid())
		{
			UE_LOG(LogBaseGauntlets, Error, TEXT("First variable pointer was never initialized or deleted in ABaseGauntlets::BeginPlay timer lambda!"));
			return;
		}

		First->State = StatePunch;
		if (void (ABaseGauntlets::*Start)(FGauntlet&) = First->State.Start)
		{
	    	(this->*Start)(*First.Get());
		}
		
	}, Cooldown, false);
}

void ABaseGauntlets::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (auto Update = Left.State.Update)
	{
		(this->*Update)(Left, DeltaTime);
	}

	if (auto Update = Right.State.Update)
	{
		(this->*Update)(Right, DeltaTime);
	}

	if (&Left.State == &StateCooldown || &Right.State == &StateCooldown)
	{
		ABaseBowToGun::FaceNearestEnemy(Pivot, Horde, (AActor*)Parent, Range);
	}
}

void ABaseGauntlets::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	StopWeapon();
	Super::EndPlay(EndPlayReason);
}

void ABaseGauntlets::Punch(FGauntlet& Gauntlet)
{
	++PunchCount;
	Gauntlet.BoxCollision->SetGenerateOverlapEvents(true);
	ABaseBowToGun::FaceNearestEnemy(Pivot, Horde, (AActor*)Parent, Range);

	if (&Gauntlet.Other->State == &StateCooldown && Gauntlet.Other.IsValid())
	{
		Gauntlet.Other->State = StateHitDelay;
	}
}

void ABaseGauntlets::Retreat(FGauntlet& Gauntlet)
{
	if (PunchCount < PunchCountMax && Gauntlet.Other.IsValid())
	{
		Gauntlet.Other->State = StatePunch;
	}
}

void ABaseGauntlets::MoveGauntlets(FGauntlet& Gauntlet, float DeltaTime)
{
	Gauntlet.GauntletPivot->AddLocalRotation(FQuat(0.f, 0.f, ((&Gauntlet.State == &StatePunch) ? Gauntlet.Direction : -Gauntlet.Direction) * DeltaTime, 1.f));
	
	void (ABaseGauntlets::*Exit)(FGauntlet&) = nullptr;
	float Angle = Gauntlet.GauntletPivot->GetRelativeRotation().Yaw;
	
	if (Gauntlet.Direction < 0.f)
	{
		if ((Angle <= -Range || Angle >= 0.f) && (Exit = Gauntlet.State.Exit))
		{
			(this->*Exit)(Gauntlet);
		}
	}
	else if ((Angle >= Range || Angle <= 0.f) && (Exit = Gauntlet.State.Exit))
	{
		(this->*Exit)(Gauntlet);
	}
}

void ABaseGauntlets::EndPunch(FGauntlet& Gauntlet)
{
	Gauntlet.State = StateHitDelay;
	Gauntlet.BoxCollision->SetGenerateOverlapEvents(false);
	Gauntlet.GauntletPivot->SetRelativeRotation(FRotator(0.f, 0.f, (Gauntlet.Direction < 0.f) ? -Range : Range));

	TWeakObjectPtr<FGauntlet> GauntletWeak = &Gauntlet;
	GetWorldTimerManager().SetTimer(TPunchCooldown, [this, GauntletWeak]()
	{
		if (!GauntletWeak.IsValid())
		{
			UE_LOG(LogBaseGauntlets, Error, TEXT("Gauntlet Weak Pointer was invalidated in ABaseGauntlets::EndPunch timer lambda!"));
			return;
		}

		GauntletWeak->State = StateRetreat;
		if (void (ABaseGauntlets::*Start)(FGauntlet&) = GauntletWeak->State.Start)
		{
			(this->*Start)(*GauntletWeak.Get());
		}
	}, PunchHitDelay, false);
}

void ABaseGauntlets::EndRetreat(FGauntlet& Gauntlet)
{
	Gauntlet.GauntletPivot->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	if (PunchCount >= PunchCountMax)
	{
		Gauntlet.State = StateCooldown;

		TWeakObjectPtr<FGauntlet> GauntletWeak = First;
		GetWorldTimerManager().SetTimer(TPunchCooldown, [this, GauntletWeak]()
		{
			if (!GauntletWeak.IsValid())
			{
				UE_LOG(LogBaseGauntlets, Error, TEXT("Gauntlet Weak Pointer was invalidated in ABaseGauntlets::EndRetreat timer lambda!"));
				return;
			}

			GauntletWeak->State = StatePunch;
			if (void (ABaseGauntlets::*Start)(FGauntlet&) = GauntletWeak->State.Start)
			{
				(this->*Start)(*GauntletWeak.Get());
			}
		}, Cooldown, false);
	}
	Gauntlet.State = StateHitDelay;
}

void ABaseGauntlets::StopWeapon()
{
	GetWorldTimerManager().ClearTimer(TPunchCooldown);
}