// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/BaseGauntlets.h"
#include "Components/BoxComponent.h"
#include "Enemies/HordeSubsystem.h"
#include "Weapons/GauntletData.h"
#include "Weapons/BaseBowToGun.h"
#include "Weapons/WeaponData.h"

DEFINE_LOG_CATEGORY_STATIC(LogBaseGauntlets, Log, All);

ABaseGauntlets::ABaseGauntlets()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Component Initialization
	Left                                      = CreateDefaultSubobject<UGauntletData>("Left");
	Right                                     = CreateDefaultSubobject<UGauntletData>("Right");

	Left->GauntletPivot  = GauntletPivotLeft  = CreateDefaultSubobject<USceneComponent>("GauntletPivotLeft");
	Left->Mesh           = MeshLeft           = CreateDefaultSubobject<USkeletalMeshComponent>("MeshLeft");
	Left->BoxCollision   = BoxCollisionLeft   = CreateDefaultSubobject<UBoxComponent>("BoxCollisionLeft");

	Right->GauntletPivot = GauntletPivotRight = CreateDefaultSubobject<USceneComponent>("GauntletPivotRight");
	Right->Mesh          = MeshRight          = CreateDefaultSubobject<USkeletalMeshComponent>("MeshRight");
	Right->BoxCollision  = BoxCollisionRight  = CreateDefaultSubobject<UBoxComponent>("BoxCollisionRight");

	// Component Attachment
	GauntletPivotLeft->SetupAttachment(Mesh);
	MeshLeft->SetupAttachment(GauntletPivotLeft);
	BoxCollisionLeft->SetupAttachment(MeshLeft);
	GauntletPivotRight->SetupAttachment(Mesh);
	MeshRight->SetupAttachment(GauntletPivotRight);
	BoxCollisionRight->SetupAttachment(MeshRight);

	// Collision
	BoxCollisionLeft->SetGenerateOverlapEvents(false);
	BoxCollisionRight->SetGenerateOverlapEvents(false);

	// State Machine
	StatePunch.Start.BindUFunction(this, FName("Punch"));
	StatePunch.Update.BindUFunction(this, FName("MoveGauntlets"));
	StatePunch.Exit.BindUFunction(this, FName("EndPunch"));

	StateRetreat.Start.BindUFunction(this, FName("Retreat"));
	StateRetreat.Update.BindUFunction(this, FName("MoveGauntlets"));
	StateRetreat.Exit.BindUFunction(this, FName("EndRetreat"));

	Left->State = &StateCooldown;
	Right->State = &StateCooldown;

	First = Right;

	bAbsoluteRotation = true;
}

void ABaseGauntlets::BeginPlay()
{
	Super::BeginPlay();

	if (UHordeSubsystem* H = GetWorld()->GetSubsystem<UHordeSubsystem>())
	{
		Horde = H;
	}
	else UE_LOG(LogBaseGauntlets, Error, TEXT("Failed to retrieve HordeSubystem in ABaseGauntlets!"));

	BoxCollisionLeft->OnComponentBeginOverlap.AddDynamic(this, &ABaseGauntlets::ApplyDamageCollision);
	BoxCollisionRight->OnComponentBeginOverlap.AddDynamic(this, &ABaseGauntlets::ApplyDamageCollision);

	// - Gauntlets Properties - 
	if (Data)
	{
		Range          = Data->Range;
		Height         = Data->Height;
		Radius         = Data->DistanceFromPlayer;
		Cooldown       = Data->Cooldown;
		Left->Direction = -(Right->Direction = -Data->AttackSpeed);
	}
	else
	{
		UE_LOG(LogBaseGauntlets, Error, TEXT("ABaseGauntlets' WeaponData asset was not set!"));

		Range    = 0.f;
		Height   = 0.f;
		Radius   = 0.f;
		Cooldown = 0.f;
		Left->Direction = Right->Direction = 0.f;
	}

	// - Location -	

	// Radius and Width (DistanceBetweenGauntlets)
	Left->GauntletPivot->AddLocalOffset(FVector(DistanceBetweenGauntlets - ArcRadius, Radius, 0.f));
	Right->GauntletPivot->AddLocalOffset(FVector(-DistanceBetweenGauntlets + ArcRadius, Radius, 0.f));
	// Arc Radius
	Left->Mesh->AddLocalOffset(FVector(ArcRadius, 0.f, 0.f));
	Right->Mesh->AddLocalOffset(FVector(-ArcRadius, 0.f, 0.f));


	/* - PUNCHING STATES -
	*  
	*  Cooldown State: Longer state of rest before restarting a cycle of punches
	* 
	*  Punch State: Active state of punching; forwards swinging motion
	* 
	*  Hit Delay State: Shorter state of rest before activating a punch or retreat
	* 
	*  Retreat State: Active state of retrieving a punch; backwards swinging motion
	* 
	*  State change for an individual Gauntlet
	*  [Cooldown -> Punching -> Hit Delay -> Retreat -> Hit Delay -> Punching again for PunchCountMax number of times -> [...] -> Retreat -> Cooldown]
	*/

	// Starts the first punch
	GetWorldTimerManager().SetTimer(TPunchCooldown, [this]()
	{
		SetState(First, &StatePunch);
	}, Cooldown, false);
}

void ABaseGauntlets::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// We call update if an update function is bound to the current state
	Left->State->Update.ExecuteIfBound(Left, DeltaTime);
	Right->State->Update.ExecuteIfBound(Right, DeltaTime);

	// Only face the enemy when both Gunatlets are in cooldown
	if (Left->State == &StateCooldown && Right->State == &StateCooldown)
	{
		ABaseBowToGun::FaceNearestEnemy(Mesh, Horde, (AActor*)Parent, EnemyFacingReach, true);
	}
}

void ABaseGauntlets::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	StopWeapon();
	Super::EndPlay(EndPlayReason);
}

void ABaseGauntlets::Punch(UGauntletData* Gauntlet)
{
	// Start of the punch. 
	// We activate overlap events to apply damage to each enemy 
	// and face the gauntlets to the nearest enemy (only once!) before initiating the punch
	++PunchCount;
	Gauntlet->BoxCollision->SetGenerateOverlapEvents(true);
	ABaseBowToGun::FaceNearestEnemy(Mesh, Horde, (AActor*)Parent, EnemyFacingReach, true);
}

void ABaseGauntlets::SetState(UGauntletData* Gauntlet, FGauntletState* State)
{
	Gauntlet->State = State;
	Gauntlet->State->Start.ExecuteIfBound(Gauntlet);
}

void ABaseGauntlets::Retreat(UGauntletData* Gauntlet)
{
	// Start of the retreat.
	if (PunchCount < PunchCountMax) // Do we still have punches left?
	{
		SetState(GetOther(Gauntlet), &StatePunch); // Yes, activate the other gauntlet for punching
	}
	else // No, initiate the cooldown protocol for both gauntlets
	{ // (this gauntlet will finish its retreat state to then go to the cooldown state
		GetWorldTimerManager().ClearTimer(TPunchCooldown);
		GetWorldTimerManager().SetTimer(TPunchCooldown, [this, Gauntlet]()
		{
			PunchCount = 0;
			SetState(First, &StatePunch);
		}, Cooldown, false);
	}
}

void ABaseGauntlets::MoveGauntlets(UGauntletData* Gauntlet, float DeltaTime)
{
	// Updates the positioning of the Gauntlet weapons
	Gauntlet->GauntletPivot->AddLocalRotation(FQuat(0.f, 0.f, ((Gauntlet->State == &StatePunch) ? Gauntlet->Direction : -Gauntlet->Direction) * DeltaTime, 1.f));
	
	float Angle = Gauntlet->GauntletPivot->GetRelativeRotation().Yaw;
	
	if (Gauntlet->Direction < 0.f) // Is this using Right's direction?
	{
		if ((Angle <= -Range || Angle >= 0.f)) // Yes! Did it go past its range?
		{
			Gauntlet->State->Exit.ExecuteIfBound(Gauntlet); // Yep! Call the exit function
		}
	}
	else if ((Angle >= Range || Angle <= 0.f)) // No, it's left! Did it go past its range?
	{
		Gauntlet->State->Exit.ExecuteIfBound(Gauntlet); // Yep! Call the exit function
	}
}

void ABaseGauntlets::EndPunch(UGauntletData* Gauntlet)
{
	// End of the punch.
	// We disable overlap events to prevent applying damage to enemies while not actively punching,
	// clamp the rotation of the Gauntlet, and remove the HitActors to ensure 
	// we can punch the same enemies with another punch.
	HitActors.Empty();
	Gauntlet->BoxCollision->SetGenerateOverlapEvents(false);
	Gauntlet->GauntletPivot->SetRelativeRotation(FRotator(0.f, (Gauntlet->Direction < 0.f) ? -Range : Range, 0.f));

	// StateHitDelay will be active and keep the Gauntlet dormant until the
	// delay timer runs out and activates this Gauntlet's retreat procedure
	SetState(Gauntlet, &StateHitDelay);

	GetWorldTimerManager().SetTimer(TPunchCooldown, [this, Gauntlet]()
	{
		SetState(Gauntlet, &StateRetreat);
	}, PunchHitDelay, false);
}
void ABaseGauntlets::EndRetreat(UGauntletData* Gauntlet)
{
	// End of the retreat.

	Gauntlet->GauntletPivot->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	if (PunchCount >= PunchCountMax)
	{
		Gauntlet->State = &StateCooldown;
	}
	else
	{
		Gauntlet->State = &StateHitDelay;
	}
}

void ABaseGauntlets::ApplyDamageCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                                      int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HitActors.Contains(OtherActor))
	{
		HitActors.Add(OtherActor);
		Super::ApplyDamage(this, OtherActor);
	}
}

void ABaseGauntlets::StopWeapon()
{
	Super::StopWeapon();

	GetWorldTimerManager().ClearTimer(TPunchCooldown);
}