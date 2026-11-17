// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/BaseSword.h"
#include "Components/BoxComponent.h"
#include "Weapons/WeaponData.h"
#include "Utility/PlayerStatsComponent.h"
#include "NiagaraComponent.h"
#include "Debug/DebugOverlaySubsystem.h"
#include "GameFramework/CheatManagerDefines.h"

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

#if UE_WITH_CHEAT_MANAGER
    if (UDebugOverlaySubsystem* Ovl = GetWorld()->GetSubsystem<UDebugOverlaySubsystem>())
    {
        Ovl->AttachToOverlay("Sword", "Type", "EvolutionLevel", FOnOverlayUpdateInt32::CreateUFunction(this, "GetEvolutionLevel"));

        Ovl->AttachToOverlay("Sword", "Components", "Height", FOnOverlayUpdateFloat::CreateUFunction(this, "GetHeight"));
        Ovl->AttachToOverlay("Sword", "Components", "Radius", FOnOverlayUpdateFloat::CreateUFunction(this, "GetRadius"));
        Ovl->AttachToOverlay("Sword", "Components", "CollisionHeightOffset", FOnOverlayUpdateFloat::CreateUFunction(this, "GetCollisionHeight"));

        Ovl->AttachToOverlay("Sword", "Swing", "IsHidden", FOnOverlayUpdateBool::CreateUFunction(this, "IsHidden"));
        Ovl->AttachToOverlay("Sword", "Swing", "IsSwinging", FOnOverlayUpdateBool::CreateUFunction(this, "IsSwinging"));
        Ovl->AttachToOverlay("Sword", "Swing", "SwingSpeed", FOnOverlayUpdateFloat::CreateUFunction(this, "GetSwingSpeed"));
        Ovl->AttachToOverlay("Sword", "Swing", "SwingArc", FOnOverlayUpdateFloat::CreateUFunction(this, "GetSwingArc"));
        Ovl->AttachToOverlay("Sword", "Swing", "SwingInterval", FOnOverlayUpdateFloat::CreateUFunction(this, "GetSwingInterval"));
        Ovl->AttachToOverlay("Sword", "Swing", "DisappearingDelay", FOnOverlayUpdateFloat::CreateUFunction(this, "GetDisappearingDelay"));
        Ovl->AttachToOverlay("Sword", "Swing", "Cooldown", FOnOverlayUpdateFloat::CreateUFunction(this, "GetCooldown"));
        Ovl->AttachToOverlay("Sword", "Swing", "DisappearingDistance", FOnOverlayUpdateFloat::CreateUFunction(this, "GetDisappearingDistance"));
        Ovl->AttachToOverlay("Sword", "Swing", "SwingCountMax", FOnOverlayUpdateInt32::CreateUFunction(this, "GetSwingCountMax"));
        Ovl->AttachToOverlay("Sword", "Swing", "SwingCount", FOnOverlayUpdateInt32::CreateUFunction(this, "GetSwingCount"));
        Ovl->AttachToOverlay("Sword", "Swing", "SwingAngle", FOnOverlayUpdateFloat::CreateUFunction(this, "GetSwingAngle"));
        Ovl->AttachToOverlay("Sword", "Swing", "PastRangeLeft", FOnOverlayUpdateFloat::CreateUFunction(this, "GetPastRangeLeft"));
        Ovl->AttachToOverlay("Sword", "Swing", "PastRangeRight", FOnOverlayUpdateFloat::CreateUFunction(this, "GetPastRangeRight"));
        Ovl->AttachToOverlay("Sword", "Swing", "Prev", FOnOverlayUpdateFloat::CreateUFunction(this, "GetPrev"));
        Ovl->AttachToOverlay("Sword", "Swing", "Next", FOnOverlayUpdateFloat::CreateUFunction(this, "GetNext"));
        Ovl->AttachToOverlay("Sword", "Swing", "Difference", FOnOverlayUpdateFloat::CreateUFunction(this, "GetDifference"));
        Ovl->AttachToOverlay("Sword", "Swing", "HitCount", FOnOverlayUpdateInt32::CreateUFunction(this, "GetHitCount"));
    }
#endif

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

        SwingPatternCooldown = Data->Cooldown;
        EvolutionLevel       = Data->EvolutionLevel;
        SwingArc             = Data->Range;
        SwingSpeed           = Data->AttackSpeed;
        Height               = Data->Height;
        Radius               = Data->DistanceFromPlayer;
    }
    else
    {
        UE_LOG(LogBaseSword, Error, TEXT("ABaseSword is missing its Data Asset!"));

        SwingPatternCooldown  = MAX_FLT;
        EvolutionLevel        = 0.f;
        SwingArc              = 0.f;
        SwingSpeed            = 0.f;
        Height                = 0.f;
        Radius                = 0.f;
    }

    PastRangeRight            = (EvolutionLevel > 0) ? SwingArc * 0.5f + 180 : SwingArc * 0.5f;
    PastRangeLeft             = (EvolutionLevel > 0) ? PastRangeRight - 360 : PastRangeRight;
    SwingDisappearingDelay    = (SwingDisappearingDelay < SwingPatternCooldown) ? SwingDisappearingDelay : 0.f;

    // Collision
    BoxCollision->SetGenerateOverlapEvents(false);
    SetActorEnableCollision(false);

    // Location
    Mesh->SetRelativeLocation(FVector(0.f, Radius, Height));
    BoxCollision->SetRelativeLocation(FVector(0.f, Radius, Height + CollisionHeightOffset));
    Pivot->SetRelativeRotation(FRotator(0.f, -PastRangeLeft, 0.f));
    bIsHidden = false;
}

void ABaseSword::Tick(float DeltaTime)
{
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

    // Did we complete the swing?
    if (SwingAngle < -PastRangeLeft) // Yes, clamp the position
    {
        Pivot->SetRelativeRotation(FRotator(0.f, -PastRangeLeft, 0.f));
        SwingAngle = -PastRangeLeft;
    }
    else if (SwingAngle > PastRangeRight) // Yes, clamp the position
    {
        Pivot->SetRelativeRotation(FRotator(0.f, PastRangeRight, 0.f));
        SwingAngle = PastRangeRight;
    }
    else return; // We have not completed the swing.

    // We have completed the swing!
    ++CurrentSwingCount;
    StopSwing();

    // Did we reach the max number of swings per set?
    if (CurrentSwingCount >= SwingCountMax) // Yes, delay the sword for the next set of swings
    {

        if (SwingCountMax % 2)
        {
            SwingAngle = PastRangeRight;
        }

        GetWorldTimerManager().SetTimer(TDisappearDelay, [this]()
            {
                NiagaraComponent->Deactivate();
                HideSword(true);
            }, SwingDisappearingDelay, false);

        GetWorldTimerManager().SetTimer(TSwingStart, [this]()
            {
                CurrentSwingCount = 0;
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

void ABaseSword::Swing(bool bRight, float DeltaTime) 
{
    Prev = Pivot->GetRelativeRotation().Yaw;
    Pivot->AddLocalRotation(FQuat(0.f, 0.f, (bRight ? -SwingSpeed * ParentStats->GetAttackSpeed() : SwingSpeed * ParentStats->GetAttackSpeed()) * DeltaTime, 1.f));
    Next = Pivot->GetRelativeRotation().Yaw;

    Difference = Next - Prev;
    if (FMath::Abs(Difference) >= 180)
    {
        SwingAngle = (bRight) ? SwingAngle - 360 : SwingAngle + 360;
    }
    SwingAngle += Difference;
}

void ABaseSword::HideSword(bool bHide)
{
    if (bIsHidden == bHide) return;
    bIsHidden = bHide;

    Pivot->AddLocalOffset(FVector(0.f, 0.f, bHide ? SwingDisappearingDistance : -SwingDisappearingDistance));
}

void ABaseSword::StartSwing()
{
    HitActors.Empty();
    bIsSwinging = true;
    OnSwingStart.Broadcast();
    SetActorTickEnabled(true);
    SetActorEnableCollision(true);
    NiagaraComponent->Activate(true);
    BoxCollision->SetGenerateOverlapEvents(true);
}

void ABaseSword::StopSwing()
{
    bIsSwinging = false;
    OnSwingEnd.Broadcast();
    SetActorTickEnabled(false);
    SetActorEnableCollision(false);
    BoxCollision->SetGenerateOverlapEvents(false);
}

void ABaseSword::StartWeapon()
{
    Super::StartWeapon();

    // Collision
    BoxCollision->SetGenerateOverlapEvents(false);
    SetActorEnableCollision(false);

    // Location
    Mesh->SetRelativeLocation(FVector(0.f, Radius, Height));
    BoxCollision->SetRelativeLocation(FVector(0.f, Radius, Height + CollisionHeightOffset));
    Pivot->SetRelativeRotation(FRotator(0.f, PastRangeRight, 0.f));
    SwingAngle = PastRangeRight;
    HideSword(false);

    StartSwing();
}

void ABaseSword::StopWeapon()
{
    StopSwing();
    HitActors.Empty();
    NiagaraComponent->Deactivate();
    GetWorldTimerManager().ClearTimer(TSwingStart);
    GetWorldTimerManager().ClearTimer(TDisappearDelay);

    HideSword(true);
 
    Super::StopWeapon();
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