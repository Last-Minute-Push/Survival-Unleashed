// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/BaseGauntlets.h"
#include "Components/BoxComponent.h"
#include "Weapons/WeaponData.h"

DEFINE_LOG_CATEGORY_STATIC(LogGauntlets, Log, All);

ABaseGauntlets::ABaseGauntlets()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Initialization
	Left.GauntletPivot = CreateDefaultSubobject<USceneComponent>("GauntletPivotLeft");
	Left.Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("MeshLeft");
	Left.BoxCollision = CreateDefaultSubobject<UBoxComponent>("BoxCollisionLeft");

	Right.GauntletPivot = CreateDefaultSubobject<USceneComponent>("GauntletPivotRight");
	Right.Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("MeshRight");
	Right.BoxCollision = CreateDefaultSubobject<UBoxComponent>("BoxCollisionRight");

	// Attachment
	Left.GauntletPivot->SetupAttachment(Pivot);
	Left.Mesh->SetupAttachment(Left.GauntletPivot);
	Left.BoxCollision->SetupAttachment(Left.Mesh);

	Right.GauntletPivot->SetupAttachment(Pivot);
	Right.Mesh->SetupAttachment(Right.GauntletPivot);
	Right.BoxCollision->SetupAttachment(Right.Mesh);

	bAbsoluteRotation = true;
}

void ABaseGauntlets::BeginPlay()
{
	// Component not needed
	Mesh->UnregisterComponent();
	Mesh->DestroyComponent();

	// - Gauntlets Properties - 
	if (Data)
	{
		Range          = Data->Range / ArcRadius + 1;
		Height         = Data->Height;
		Radius         = Data->DistanceFromPlayer;
		Cooldown       = Data->Cooldown;
		Left.Direction = -(Right.Direction = Data->AttackSpeed / ArcRadius + 1);
	}
	else
	{
		UE_LOG(LogGauntlets, Error, TEXT("ABaseGauntlets' WeaponData asset was not set!"));

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

	bIsAiming = true;

	GetWorldTimerManager().SetTimer(TPunchCooldown, [this]()
		{
			Punch(Left);
		}, Cooldown, false);
}

void ABaseGauntlets::Tick(float DeltaTime)
{
	if (bIsAiming)
	{
		ABaseBowToGun::FaceNearestEnemy(Pivot, Horde, (AActor*)Parent, Range);
		return;
	}

	if (!Left.bIsPunching && !Right.bIsPunching)
	{
		EndCycle();
	}

	MoveGauntlets();

	float LeftAngle = Left.GauntletPivot->GetRelativeRotation().Yaw;
	float RightAngle = Right.GauntletPivot->GetRelativeRotation().Yaw;

	if (RightAngle >= Range)
	{
		Right.bIsActive = false;

		Right.BoxCollision->SetGenerateOverlapEvents(false);
		GetWorldTimerManager().SetTimer(TPunchCooldown, [this]()
			{
				Retreat(Right);
			}, AfterPunchDelay, false);
	}
	else if (RightAngle <= 0)
	{
		Right.bIsActive = false;
	}

	if (LeftAngle < -Range)
	{
		Left.bIsActive = false;

		Left.BoxCollision->SetGenerateOverlapEvents(false);
		GetWorldTimerManager().SetTimer(TPunchCooldown, [this]()
			{
				Retreat(Left);
			}, AfterPunchDelay, false);
	}
	else if (LeftAngle >= 0)
	{
		Left.bIsActive = false;
	}
}

void ABaseGauntlets::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	StopWeapon();
	Super::EndPlay(EndPlayReason);
}

void ABaseGauntlets::Punch(FGauntlet& Gauntlet)
{
	bIsAiming = false;
	Gauntlet.bIsActive = true;
	Gauntlet.bIsPunching = true;
	Gauntlet.BoxCollision->SetGenerateOverlapEvents(true);
}

void ABaseGauntlets::Retreat(FGauntlet& Gauntlet)
{
	ABaseBowToGun::FaceNearestEnemy(Pivot, Horde, (AActor*)Parent, Range);

	Gauntlet.bIsActive = true;
	Gauntlet.bIsPunching = false;
	if (PunchCount < PunchCountMax)
	{
		Punch(GetOther(Gauntlet));
	}
}

void ABaseGauntlets::EndCycle()
{
	bIsAiming = true;
	Left.bIsActive = false;
	Right.bIsActive = false;
}

void ABaseGauntlets::MoveGauntlets()
{
	if (Left.bIsActive)
	{
		Left.GauntletPivot->AddLocalRotation(FQuat(0.f, 0.f, (Left.bIsPunching) ? Left.Direction : -Left.Direction, 1.f));
	}
	if (Right.bIsActive)
	{
		Right.GauntletPivot->AddLocalRotation(FQuat(0.f, 0.f, (Right.bIsPunching) ? Right.Direction : -Right.Direction, 1.f));
	}
}

void ABaseGauntlets::StopWeapon()
{
	GetWorldTimerManager().ClearTimer(TPunchCooldown);

	bIsAiming = Left.bIsActive = Right.bIsActive = false;
}