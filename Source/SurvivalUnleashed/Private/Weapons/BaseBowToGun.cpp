// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/BaseBowToGun.h"
#include "Weapons/ProjectileBase.h"
#include "Weapons/WeaponData.h"
#include "Enemies/EnemyBase.h"
#include "Actor/UC_Player.h"

DEFINE_LOG_CATEGORY_STATIC(LogBowToGun, Log, All);

ABaseBowToGun::ABaseBowToGun()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Initialization
	DisplayProjectile = CreateDefaultSubobject<USkeletalMeshComponent>("DisplayProjectile");

	// Bow Properties
	Height                       = 75.f;
	Range                        = 140.f;
	ProjectileDisappearingHeight = 1000.f;

    // Attachments
	DisplayProjectile->SetupAttachment(Mesh);

	bAbsoluteRotation = true;
}

void ABaseBowToGun::BeginPlay()
{
	Super::BeginPlay();

	if (UHordeSubsystem* H = GetWorld()->GetSubsystem<UHordeSubsystem>())
	{
		Horde = H;
	}
	else UE_LOG(LogBowToGun, Error, TEXT("Failed to retrieve HordeSubystem in ABaseBowToGun!"));

	// Bow Properties
	if (Data)
	{
		Range     = Data->Range;
		Damage    = Data->Damage;
		Height    = Data->Height;
		Radius    = Data->DistanceFromPlayer;
		Cooldown  = Data->Cooldown * 0.5f;
		Knockback = Data->Knockback;
		Speed     = Data->AttackSpeed;
		Pierce    = Data->ProjectilePierce;
	}
	else
	{
		UE_LOG(LogBowToGun, Error, TEXT("ABaseBowToGun's WeaponData asset was not set!"));

		Range = 0.f;
		Damage = 0.f;
		Cooldown = 0.f;
		Knockback = 0.f;
	}

	// Location
	UE_LOG(LogBowToGun, Error, TEXT("HEIGHT: %f"), Height);
	Mesh->AddLocalOffset(FVector(0.f, Radius, Height));
	Mesh->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));

	HideProjectile(true);
	GetWorldTimerManager().SetTimer(TFireCooldown, this, &ABaseBowToGun::StartAim, Cooldown);
}

void ABaseBowToGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FaceNearestEnemy(Mesh/*, false*/);
}

void ABaseBowToGun::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(TFireCooldown);
	GetWorldTimerManager().ClearTimer(TAimDelay);

	Super::EndPlay(EndPlayReason);
}

bool ABaseBowToGun::FaceNearestEnemy(USceneComponent* ToRotate, bool bFlipRotation/*= false*/, bool bForce/*= true*/)
{
	AEnemyBase* Nearest = Horde->FindNearestEnemy(Parent->GetActorLocation(), Range);

	if (!Nearest)
	{
		return false;
	}

	// We create a normalized directional vector
	FRotator Direction = FVector(Parent->GetActorLocation() - Nearest->GetActorLocation()).GetSafeNormal2D().Rotation();
	
	if (Direction.IsNearlyZero())
	{
		return false;
	}

	// We then find the angle in the directional vector and apply the rotation to the component
	if (bForce)
	{
		Direction.Yaw += bFlipRotation ? 90.f : -90.f;
		ToRotate->SetWorldRotation(Direction);
	}
	else
	{
	    // maybe rotate it gradually (using add and not set) instead of instantly snapping into position?
	    // linearly slowing down the rotation speed would look even fancier...
	}
	
	return true;
}

void ABaseBowToGun::StartAim()
{
	HideProjectile(false);

	GetWorldTimerManager().SetTimer(TAimDelay, this, &ABaseBowToGun::StopAim, Cooldown);
}

void ABaseBowToGun::StopAim()
{
	// Projectile Properties
	FVector SpawnLocation = GetActorLocation();

	FRotator SpawnDirection = Mesh->GetRelativeRotation();
	SpawnDirection.Yaw += 180.f;

	FProjectileLaunchParams Params;
	Params.Origin = SpawnLocation;
	Params.Direction = SpawnDirection.Vector();
	Params.Speed = Speed * ParentStats->GetAttackSpeed();
	Params.Pierce = Pierce;
	Params.Range = Range;

	FOnProjectileHit Hit;
	Hit.BindUFunction(this, TEXT("ApplyProjectileDamage"));

	// Launch the projectile.
	AProjectileBase::Fire(GetWorld(), Data->ProjectileType, this, Params, Hit);

	HideProjectile(true);

	GetWorldTimerManager().SetTimer(TFireCooldown, this, &ABaseBowToGun::StartAim, Cooldown);
}

void ABaseBowToGun::StopWeapon()
{
	GetWorldTimerManager().ClearTimer(TAimDelay);
	GetWorldTimerManager().ClearTimer(TFireCooldown);

	Super::StopWeapon();
}