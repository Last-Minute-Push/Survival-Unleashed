// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/BaseSpear.h"
#include "Weapons/WeaponData.h"
#include "Components/BoxComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpear, Log, All);

ABaseSpear::ABaseSpear()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	BoxCollision = CreateDefaultSubobject<UBoxComponent>("BoxCollision");
	BoxCollision->AttachToComponent(Mesh, FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false));
}

void ABaseSpear::BeginPlay()
{
	Super::BeginPlay();

	// Delegate Binding
	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ABaseSpear::ApplyDamageCollision);

	// Thrust Properties
	if (Data)
	{
		Cooldown = Data->Cooldown;
		Range   += Radius;
	}
	else
	{
		UE_LOG(LogSpear, Error, TEXT("ABaseSpear's WeaponData asset was not set!"))
	}

	bIsAiming = true;

	// Begins the thrust cycle	
	GetWorldTimerManager().ClearTimer(TFireCooldown);
	GetWorldTimerManager().SetTimer(TFireCooldown, this, &ABaseSpear::StartThrust, Cooldown);
}

void ABaseSpear::Tick(float DeltaTime)
{
	// We only aim while we're not thrusting
	if (bIsAiming)
	{
		FaceNearestEnemy(Pivot, true/*, false*/);
		return;
	}

	// Thrusting movement
	float CurrentThrustDistance = Mesh->GetRelativeLocation().Y;
	Mesh->AddRelativeLocation(FVector(0.f, (bIsThrusting) ? Speed * DeltaTime : -Speed * ThrustRetreatMultiplier * DeltaTime, 0.f));

	// Did we reach our range?
	if (bIsThrusting && CurrentThrustDistance > Range)
	{
		StopThrust(); // Yes, we fully thrusted our spear
	}
	else if (CurrentThrustDistance < Radius)
	{
		EndThrustCycle(); // Yes, we fully retreated our spear 
	}
	// else we continue the movement
}

void ABaseSpear::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopWeapon();

	Super::EndPlay(EndPlayReason);
}

void ABaseSpear::StartThrust()
{
	bIsAiming = false;
	bIsThrusting = true;

	BoxCollision->SetGenerateOverlapEvents(true);
}

void ABaseSpear::StopThrust()
{
	bIsThrusting = false;
	Mesh->SetRelativeLocation(FVector(0.f, Range, Height));
	BoxCollision->SetGenerateOverlapEvents(false);
}

void ABaseSpear::EndThrustCycle()
{
	HitActors.Empty();

	bIsAiming = true;
	bIsThrusting = true;
	Mesh->SetRelativeLocation(FVector(0.f, Radius, Height));

	GetWorldTimerManager().SetTimer(TFireCooldown, this, &ABaseSpear::StartThrust, Cooldown);
}

void ABaseSpear::ApplyDamageCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsThrusting && !HitActors.Contains(OtherActor))
	{
		HitActors.Add(OtherActor);
		ApplyDamage(this, OtherActor);
	}
}

void ABaseSpear::StopWeapon()
{
	HitActors.Empty();

	Super::StopWeapon();
}