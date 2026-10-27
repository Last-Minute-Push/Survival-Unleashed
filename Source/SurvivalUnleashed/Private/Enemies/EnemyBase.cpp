// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/EnemyBase.h"
#include "Enemies/EnemyData.h"
#include "UI/DamageNumbersUI.h"
#include "Framework/ObjectPoolSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Enemies/HordeSubsystem.h"
#include "Actor/EXP_PickUp.h"
#include "Debug/SUDebug.h"
#include "DrawDebugHelpers.h"
#include "Audio/Interfaces/AudioManagerInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnemy, Log, All);

float AEnemyBase::LargestSeparationRadius = 0.f;

// Sets default values
AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(40.f, 90.f);
	Capsule->SetCollisionProfileName(TEXT("Enemy"));
	Capsule->SetGenerateOverlapEvents(true);
	RootComponent = Capsule;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Capsule);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (!EnemyData)
	{
		UE_LOG(LogEnemy, Warning, TEXT("%s has no EnemyData assigned"), *GetName());
	}

	Capsule->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::OnCapsuleBeginOverlap);
	Capsule->OnComponentEndOverlap.AddDynamic(this, &AEnemyBase::OnCapsuleEndOverlap);

	TInlineComponentArray<UMeshComponent*> Meshes(this);
	MeshComponents.Reset();
	for (UMeshComponent* MeshComp : Meshes)
	{
		MeshComponents.Add(MeshComp);
	}

	Horde = GetWorld()->GetSubsystem<UHordeSubsystem>();
	if (Horde)
	{
		Horde->RegisterEnemy(this);
	}

	LargestSeparationRadius = FMath::Max(LargestSeparationRadius, SeparationRadius);

	ResetState();
}

void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Horde)
	{
		Horde->UnregisterEnemy(this);
	}

	Super::EndPlay(EndPlayReason);
}

float AEnemyBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	//SoundEffect played...
	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{
		if (I->GetLibrary()->SFX_EnemyHurt) {
			I->PlaySound(I->GetLibrary()->SFX_EnemyHurt);
		}
	}

	if (bIsDead)
	{
		return 0.f;
	}

	const float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (Damage <= 0.f)
	{
		return 0.f;
	}

	EnemyFlash();
	CurrentHealth -= Damage;
	OnHit.Broadcast(this, Damage);

	if (CurrentHealth <= 0.f)
	{
		Die();
	}

	return Damage;
}

void AEnemyBase::OnPoolAcquire_Implementation()
{
	ResetState();
}

void AEnemyBase::OnPoolRelease_Implementation()
{
	bIsDead = true;
	GetWorldTimerManager().ClearTimer(ContactDamageHandle);
}

void AEnemyBase::ResetState()
{
	bIsDead = false;
	CurrentHealth = EnemyData ? EnemyData->MaxHealth : 1.f;
	DamageMultiplier = 1.f;
	CurrentVelocity = FVector::ZeroVector;
	KnockbackVelocity = FVector::ZeroVector;
	KnockbackTimeRemaining = 0.f;

	MoveAmount = 0.f;
	LastSentMoveAmount = 0.f;
	SetMaterialData(EnemyMaterialData::AnimPhase, FMath::FRandRange(0.f, UE_TWO_PI));
	SetMaterialData(EnemyMaterialData::MoveAmount, 0.f);
	SetMaterialData(EnemyMaterialData::HitFlash, 0.f);
	ClearStatuses();

	GetWorldTimerManager().ClearTimer(ContactDamageHandle);
}

void AEnemyBase::Die()
{
	bIsDead = true;
	GetWorldTimerManager().ClearTimer(ContactDamageHandle);

	// Broadcast before releasing so listeners can still read location and data
	OnDied.Broadcast(this);
	DropLoot();

	if (UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
	{
		Pool->Release(this);
	}
}

void AEnemyBase::DealContactDamage()
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (bIsDead || !Player || !EnemyData || IsDisabled())
	{
		return;
	}

	UGameplayStatics::ApplyDamage(Player, EnemyData->ContactDamage * DamageMultiplier, nullptr, this, nullptr);
}

void AEnemyBase::ApplyDifficulty(float InHealthMultiplier, float InDamageMultiplier)
{
	DamageMultiplier = FMath::Max(InDamageMultiplier, 0.f);
	CurrentHealth = (EnemyData ? EnemyData->MaxHealth : 1.f) * FMath::Max(InHealthMultiplier, 0.01f);
}

void AEnemyBase::ApplyKnockback(const FVector& SourceLocation, float Distance)
{
	if (bIsDead || Distance <= 0.f)
	{
		return;
	}

	const float Effective = Distance * (1.f - EnemyData->KnockbackResistance);
	if (Effective <= KINDA_SMALL_NUMBER)
	{
		return; // immune
	}

	FVector Direction = GetActorLocation() - SourceLocation;
	Direction.Z = 0.f;
	Direction = Direction.GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = -GetActorForwardVector(); // source exactly on top of us: push backward
	}

	// Speed starts at 2x average and fades to 0, so the total distance still comes out to Effective
	KnockbackVelocity = Direction * (2.f * Effective / EnemyData->KnockbackDuration);
	KnockbackTimeRemaining = EnemyData->KnockbackDuration; // a new hit replaces any knockback in progress
}

void AEnemyBase::ApplyStatus(EEnemyStatus Status, float Duration, float Magnitude, AActor* Source, FVector PullCenter)
{
	if (bIsDead || Duration <= 0.f || Status == EEnemyStatus::Count)
	{
		return;
	}
	if (Status == EEnemyStatus::Pull && EnemyData && EnemyData->bIgnoresPull)
	{
		return;
	}

	FStatusState& S = Statuses[static_cast<int32>(Status)];
	// Refresh, don't stack: keep the longer duration and the stronger effect
	S.TimeRemaining = FMath::Max(S.TimeRemaining, Duration);
	S.Magnitude = FMath::Max(S.Magnitude, Magnitude);
	S.Causer = Source;
	S.Instigator = Source ? Source->GetInstigatorController() : nullptr;
	if (Status == EEnemyStatus::Pull)
	{
		S.PullCenter = PullCenter; // the latest tornado wins
	}
	RefreshStatusTint();
}

bool AEnemyBase::IsStatusActive(EEnemyStatus Status) const
{
	return Status != EEnemyStatus::Count && Statuses[static_cast<int32>(Status)].TimeRemaining > 0.f;
}

void AEnemyBase::UpdateStatuses(float DeltaTime)
{
	bool bChanged = false;
	for (int32 i = 0; i < static_cast<int32>(EEnemyStatus::Count); ++i)
	{
		FStatusState& S = Statuses[i];
		if (S.TimeRemaining <= 0.f)
		{
			continue;
		}

		const float Step = FMath::Min(DeltaTime, S.TimeRemaining);
		S.TimeRemaining -= Step;

		// Burn and Freeze deal damage every 0.5 s. The small tolerance keeps float drift
		// from dropping the last tick (3 s of burn = exactly 6 ticks).
		const bool bDealsDamage = (i == static_cast<int32>(EEnemyStatus::Burn) || i == static_cast<int32>(EEnemyStatus::Freeze));
		if (bDealsDamage && S.Magnitude > 0.f)
		{
			S.TickAccumulator += Step;
			while (S.TickAccumulator >= StatusTickInterval - KINDA_SMALL_NUMBER)
			{
				S.TickAccumulator -= StatusTickInterval;

				// Through TakeDamage with the weapon as causer, so kills, drops, hit flash, and damage numbers all work
				TakeDamage(S.Magnitude, FDamageEvent(UDamageType::StaticClass()), S.Instigator.Get(), S.Causer.Get());
				if (bIsDead)
				{
					return; // released to the pool; ResetState will clear the rest on reuse
				}
			}
		}
		if (S.TimeRemaining <= 0.f)
		{
			S = FStatusState();
			bChanged = true;
		}
	}
	if (bChanged)
	{
		RefreshStatusTint();
	}
}

void AEnemyBase::ClearStatuses()
{
	for (FStatusState& S : Statuses)
	{
		S = FStatusState();
	}
	LastTintStatus = INDEX_NONE;
	SetMaterialColor(EnemyMaterialData::StatusTint, FLinearColor::Transparent); // (0,0,0,0) = no tint
}

void AEnemyBase::RefreshStatusTint()
{
	// One tint at a time: Freeze reads strongest, then Burn, then Stun
	int32 TintStatus = INDEX_NONE;
	FLinearColor Tint = FLinearColor::Transparent;
	if (IsStatusActive(EEnemyStatus::Freeze)) { TintStatus = static_cast<int32>(EEnemyStatus::Freeze); Tint = FreezeTint; }
	else if (IsStatusActive(EEnemyStatus::Burn)) { TintStatus = static_cast<int32>(EEnemyStatus::Burn);   Tint = BurnTint; }
	else if (IsStatusActive(EEnemyStatus::Stun)) { TintStatus = static_cast<int32>(EEnemyStatus::Stun);   Tint = StunTint; }

	// Only send to the renderer when the tint actually changes
	if (TintStatus != LastTintStatus)
	{
		LastTintStatus = TintStatus;
		SetMaterialColor(EnemyMaterialData::StatusTint, Tint);
	}
}

void AEnemyBase::SetMaterialColor(int32 Index, const FLinearColor& Value)
{
	for (UMeshComponent* MeshComp : MeshComponents)
	{
		if (MeshComp)
		{
			MeshComp->SetCustomPrimitiveDataVector4(Index, FVector4(Value));
		}
	}
}

void AEnemyBase::UpdateKnockback(float DeltaTime)
{
	const float Step = FMath::Min(DeltaTime, KnockbackTimeRemaining);

	// Ease-out: full speed at the hit, slowing linearly to a stop
	const float Alpha = KnockbackTimeRemaining / EnemyData->KnockbackDuration;
	CurrentVelocity = KnockbackVelocity * Alpha;
	MoveWithSlide(CurrentVelocity * Step);

	KnockbackTimeRemaining -= Step;
}

void AEnemyBase::DropLoot()
{
	if (!ExperiencePickupClass || !EnemyData || EnemyData->ExperienceValue <= 0)
	{
		return;
	}

	UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (!Pool)
	{
		return;
	}

	// The gem ignores pickups until it has a value, so setting it after Acquire is safe
	const FTransform DropTransform(FRotator::ZeroRotator, GetActorLocation());
	if (AEXP_PickUp* Gem = Pool->AcquireAs<AEXP_PickUp>(ExperiencePickupClass, DropTransform))
	{
		Gem->SetExpValue(EnemyData->ExperienceValue);
	}
}

void AEnemyBase::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsDead || OtherActor != UGameplayStatics::GetPlayerPawn(this, 0))
	{
		return;
	}

	DealContactDamage();

	const float Interval = EnemyData ? EnemyData->ContactDamageInterval : 0.5f;
	GetWorldTimerManager().SetTimer(ContactDamageHandle, this, &AEnemyBase::DealContactDamage, Interval, true);
}

void AEnemyBase::OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == UGameplayStatics::GetPlayerPawn(this, 0))
	{
		GetWorldTimerManager().ClearTimer(ContactDamageHandle);
	}
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateStatuses(DeltaTime);
	if (bIsDead)
	{
		return; // a burn or freeze tick killed us and we've been released to the pool
	}

#if ENABLE_DRAW_DEBUG
	if (SUDebug::IsAIDebugEnabled())
	{
		DrawDebugSeparation(); // before the early returns below, so knocked-back and pulled enemies still draw
	}
#endif

	// Being knocked back: skip steering until it ends
	if (KnockbackTimeRemaining > 0.f)
	{
		UpdateKnockback(DeltaTime);
		UpdateMoveAmount(DeltaTime);
		return;
	}

	// Pulled: move toward the pull center instead of the player (wins over Freeze and Stun)
	if (IsStatusActive(EEnemyStatus::Pull))
	{
		const FStatusState& Pull = Statuses[static_cast<int32>(EEnemyStatus::Pull)];
		FVector ToCenter = Pull.PullCenter - GetActorLocation();
		ToCenter.Z = 0.f;

		// Keep separation so pulled enemies ring the center instead of stacking on one point
		const FVector PullDir = (ToCenter.GetSafeNormal2D() + ComputeSeparation() * SeparationWeight).GetClampedToMaxSize2D(1.f);
		const float Step = FMath::Min(Pull.Magnitude * DeltaTime, ToCenter.Size2D());

		CurrentVelocity = PullDir * Pull.Magnitude;
		MoveWithSlide(PullDir * Step);
		UpdateMoveAmount(DeltaTime);
		return;
	}

	// Frozen or stunned: stand still
	if (IsDisabled())
	{
		CurrentVelocity = FVector::ZeroVector;
		UpdateMoveAmount(DeltaTime);
		return;
	}


	// Update movement
	const FVector Direction = (bIsDead || !EnemyData) ? FVector::ZeroVector : GetDesiredDirection();

	// If the direction is very small, stop moving to avoid jittering
	if (Direction.SizeSquared2D() < FMath::Square(0.1f))
	{
		CurrentVelocity = FVector::ZeroVector;
	}
	else
	{
		// Move the enemy in the desired direction
		CurrentVelocity = Direction * EnemyData->MoveSpeed;
		MoveWithSlide(CurrentVelocity * DeltaTime);

		// Rotate the enemy to face the desired direction
		const FRotator TargetRotation(0.f, Direction.Rotation().Yaw, 0.f);
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, TurnSpeed));
	}

	// Update the move amount for animation purposes
	UpdateMoveAmount(DeltaTime);
}

FVector AEnemyBase::GetDesiredDirection() const
{
	FVector Seek = FVector::ZeroVector;
	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
		if (ToPlayer.SizeSquared2D() > FMath::Square(StopDistance))
		{
			Seek = ToPlayer.GetSafeNormal2D();
		}
	}

	const FVector Desired = Seek * SeekWeight + ComputeSeparation() * SeparationWeight;
	return Desired.GetClampedToMaxSize2D(1.f);
}

FVector AEnemyBase::ComputeSeparation() const
{
	if (!Horde)
	{
		return FVector::ZeroVector;
	}

	const FVector MyLocation = GetActorLocation();

	// Far enough to find the biggest neighbor that could be pushing on us
	const float QueryRadius = (SeparationRadius + LargestSeparationRadius) * 0.5f;
	Horde->GetNeighbors(MyLocation, QueryRadius, NeighborScratch);

	FVector Push = FVector::ZeroVector;
	for (const AEnemyBase* Other : NeighborScratch)
	{
		if (Other == this)
		{
			continue;
		}

		FVector Away = MyLocation - Other->GetActorLocation();
		Away.Z = 0.f;
		const float Dist = Away.Size();

		// A pair starts pushing at the average of their personal-space radii
		const float PairRadius = (SeparationRadius + Other->SeparationRadius) * 0.5f;
		if (Dist >= PairRadius)
		{
			continue;
		}
		const float MassRatio = Other->Mass / Mass;

		if (Dist < KINDA_SMALL_NUMBER)
		{
			// Exactly on top of each other: push in a direction unique to this enemy
			const float Angle = FMath::DegreesToRadians(static_cast<float>(GetUniqueID() % 360));
			Push += FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * MassRatio;
			continue;
		}

		// Closer and heavier neighbors push harder; fades to 0 at the pair radius
		Push += (Away / Dist) * (1.f - Dist / PairRadius) * MassRatio;
	}

	return Push.GetClampedToMaxSize2D(1.f);
}

void AEnemyBase::EnemyFlash()
{
	SetMaterialData(EnemyMaterialData::HitFlash, 1.f);
	GetWorldTimerManager().SetTimer(HitFlashDurationHandle, this, &AEnemyBase::RevertFlash, HitFlashDuration, false);
}

void AEnemyBase::RevertFlash()
{
	SetMaterialData(EnemyMaterialData::HitFlash, 0.f);
}

void AEnemyBase::UpdateMoveAmount(float DeltaTime)
{
	const float MaxSpeed = EnemyData ? EnemyData->MoveSpeed : 0.f;
	const float Target = MaxSpeed > 0.f ? FMath::Clamp(CurrentVelocity.Size() / MaxSpeed, 0.f, 1.f) : 0.f;
	MoveAmount = FMath::FInterpTo(MoveAmount, Target, DeltaTime, MoveAmountInterpSpeed);

	// Only send to the renderer when the value has noticeably changed (for performance)
	if (FMath::Abs(MoveAmount - LastSentMoveAmount) > 0.02f)
	{
		LastSentMoveAmount = MoveAmount;
		SetMaterialData(EnemyMaterialData::MoveAmount, MoveAmount);
	}
}

void AEnemyBase::SetMaterialData(int32 Index, float Value)
{
	for (UMeshComponent* MeshComp : MeshComponents)
	{
		if (MeshComp)
		{
			MeshComp->SetCustomPrimitiveDataFloat(Index, Value);
		}
	}
}

void AEnemyBase::DrawDebugSeparation() const
{
#if ENABLE_DRAW_DEBUG
	const FVector Location = GetActorLocation();

	// Half the radius: a pair pushes at the average of both radii, so two circles touch exactly when they start pushing
	DrawDebugCircle(GetWorld(), Location, SeparationRadius * 0.5f, 24, FColor::Yellow, false, -1.f, 0, 2.f,
		FVector(1.f, 0.f, 0.f), FVector(0.f, 1.f, 0.f), false);

	// The push this enemy feels right now (debug only, so the extra neighbor query is fine)
	const FVector Push = ComputeSeparation();
	if (!Push.IsNearlyZero())
	{
		DrawDebugLine(GetWorld(), Location, Location + Push * SeparationRadius, FColor::Orange, false, -1.f, 0, 2.f);
	}
#endif
}

void AEnemyBase::MoveWithSlide(const FVector& Delta)
{
	FHitResult Hit;
	AddActorWorldOffset(Delta, true, &Hit);
	if (Hit.bBlockingHit && !Hit.bStartPenetrating)
	{
		FVector Remaining = FVector::VectorPlaneProject(Delta * (1.f - Hit.Time), Hit.Normal);
		Remaining.Z = 0.f;
		AddActorWorldOffset(Remaining, true);
	}
}