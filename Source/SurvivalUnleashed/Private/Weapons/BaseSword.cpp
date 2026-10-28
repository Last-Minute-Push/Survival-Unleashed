// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/BaseSword.h"
#include "Components/BoxComponent.h"
#include "Weapons/WeaponData.h"
#include "Utility/PlayerStatsComponent.h"
#include "NiagaraComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogBaseSword, Log, All);

ABaseSword::ABaseSword()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // Initialization
    BoxCollision = CreateDefaultSubobject<UBoxComponent>("BoxCollision");
    NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("NiagaraComponent");

    // Attachments
    BoxCollision->SetupAttachment(Pivot);
    NiagaraComponent->SetupAttachment(Mesh);

    // Swing Properties
    SwingSpeed                = 10.f;
    SwingCountMax             = 2;
    SwingInterval             = 0.25f;
    CurrentSwingCount         = 0;
    SwingDisappearingDistance = 1000.f;
    SwingDisappearingDelay    = 0.25f;
    bIsSwinging               = false;

    // Collision Properties
    CollisionHeightOffset     = -40.f;

    // Sizing
    Mesh->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.2f));
    BoxCollision->SetRelativeScale3D(FVector(0.2f, 3.f, 1.5f));
}

void ABaseSword::BeginPlay()
{
    Super::BeginPlay();

    // Delegate Binding
    BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ABaseSword::ApplyDamageCollision);

    // Swing Properties
    if (Data)
    {
        if (Data->NiagaraSystem)
        {
            NiagaraComponent->SetAsset(Data->NiagaraSystem);
            NiagaraComponent->SetVariableFloat("TrailWidth", Data->TrailWidth);
            NiagaraComponent->SetVariableFloat("TrailLifespan", Data->TrailLifespan > 0.f ? Data->TrailLifespan : SwingInterval);
            NiagaraComponent->SetVariableLinearColor("TrailColor", Data->TrailColor);
            NiagaraComponent->Deactivate();
        }
        else UE_LOG(LogBaseSword, Error, TEXT("BaseSword's WeaponData Niagara System asset was not set!"));

        SwingPatternCooldown = Data->Cooldown;
        SwingArc             = Data->Range < 360.f ? Data->Range : 360.f;
        SwingSpeed           = Data->AttackSpeed;
        Height               = Data->Height;
        Radius               = Data->DistanceFromPlayer;
    }
    else
    {
        UE_LOG(LogBaseSword, Error, TEXT("ABaseSword is missing its Data Asset!"));

        SwingPatternCooldown  = MAX_FLT;
        SwingArc              = 0.f;
        SwingSpeed            = 0.f;
        Height                = 0.f;
        Radius                = 0.f;
    }

    SwingArcHalf              = SwingArc * 0.5f;
    SwingDisappearingDelay    = SwingDisappearingDelay < SwingPatternCooldown ? SwingDisappearingDelay : 0.f;
    BoxCollision->SetGenerateOverlapEvents(false);
    SetActorEnableCollision(false);

    // Location
    Mesh->AddLocalOffset(FVector(0.f, Radius, Height));
    BoxCollision->AddLocalOffset(FVector(0.f, Radius, Height + CollisionHeightOffset));
    Pivot->AddLocalRotation(FRotator(0.f, SwingArcHalf, 0.f));

    // Starting delay
    GetWorldTimerManager().SetTimer(TSwingStart, this, &ABaseSword::StartSwing, SwingPatternCooldown);
}

void ABaseSword::Tick(float DeltaTime)
{
    bIsSwinging = true;
    Super::Tick(DeltaTime);

    // We will swing left or right
    if (CurrentSwingCount % 2)
    {
        Swing(false, DeltaTime);
    }
    else
    {
        Swing(true, DeltaTime);
    }

    SwingAngle = Pivot->GetRelativeRotation().Yaw;

    // Did we complete the swing?
    if (SwingAngle < -SwingArcHalf) // Yes, clamp the position
    {
        Pivot->SetRelativeRotation(FRotator(0.f, -SwingArcHalf, 0.f));
    }
    else if (SwingAngle > SwingArcHalf) // Yes, clamp the position
    {
        Pivot->SetRelativeRotation(FRotator(0.f, SwingArcHalf, 0.f));
    }
    else return; // We have not completed the swing.

    // We have completed the swing!
    ++CurrentSwingCount;
    StopSwing();

    // Did we reach the max number of swings per set?
    if (CurrentSwingCount >= SwingCountMax) // Yes, delay the sword for the next set of swings
    {
        CurrentSwingCount = 0;

        GetWorldTimerManager().SetTimer(TDisappearDelay, [this]()
            {
                NiagaraComponent->Deactivate();
                HideSword(true);
            }, SwingDisappearingDelay, false);

        GetWorldTimerManager().SetTimer(TSwingStart, [this]()
            {
                HideSword(false);
                StartSwing();
            }, SwingPatternCooldown, false);
    }
    else // No, continue the set of swings
    {
        GetWorldTimerManager().SetTimer(TSwingStart, this, &ABaseSword::StartSwing, SwingInterval);
    }
}

void ABaseSword::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopWeapon();

    Super::EndPlay(EndPlayReason);
}

void ABaseSword::Swing(bool bRight, float DeltaTime) const
{
    Pivot->AddLocalRotation(FQuat(0.f, 0.f, (bRight ? -SwingSpeed * ParentStats->GetAttackSpeed() : SwingSpeed * ParentStats->GetAttackSpeed()) * DeltaTime, 1.f));
}

void ABaseSword::StartSwing()
{
    HitActors.Empty();
    bIsSwinging = false;
    OnSwingStart.Broadcast();
    SetActorTickEnabled(true);
    SetActorEnableCollision(true);
    NiagaraComponent->Activate(true);
    BoxCollision->SetGenerateOverlapEvents(true);
}

void ABaseSword::StopSwing()
{
    bIsSwinging = true;
    OnSwingEnd.Broadcast();
    SetActorTickEnabled(false);
    SetActorEnableCollision(false);
    BoxCollision->SetGenerateOverlapEvents(false);
}

void ABaseSword::StopWeapon()
{
    StopSwing();
    NiagaraComponent->Deactivate();
    GetWorldTimerManager().ClearTimer(TSwingStart);
    GetWorldTimerManager().ClearTimer(TDisappearDelay);

    HideSword(true);
}

void ABaseSword::ApplyDamageCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		                     int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bIsSwinging && !HitActors.Contains(OtherActor))
    {
        HitActors.Add(OtherActor);
        ApplyDamage(this, OtherActor);
    }
}