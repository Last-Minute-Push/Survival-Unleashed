// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/EXP_PickUp.h"
#include "Actor/UC_Player.h"
#include "Components/BoxComponent.h"
#include "PaperSpriteComponent.h"
#include "Components/CapsuleComponent.h"
#include "Framework/ObjectPoolSubsystem.h"
#include "Components/SphereComponent.h"


// Sets default values
AEXP_PickUp::AEXP_PickUp()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	BoxCollision = CreateDefaultSubobject<UBoxComponent>("BoxCollision");
	RootComponent = BoxCollision;
	Sprite = CreateDefaultSubobject<UPaperSpriteComponent>("Sprite");
	Sprite2 = CreateDefaultSubobject<UPaperSpriteComponent>("Sprite2");
	Sprite3 = CreateDefaultSubobject<UPaperSpriteComponent>("Sprite3");
	Sprite->SetupAttachment(BoxCollision);
	Sprite2->SetupAttachment(BoxCollision);
	Sprite3->SetupAttachment(BoxCollision);
	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AEXP_PickUp::HandleOverLap);
	Sprite->SetVisibility(true);
	Sprite2->SetVisibility(false);
	Sprite3->SetVisibility(false);

}

void AEXP_PickUp::SetExpValue(int Amount)
{
	// The gem may have been switched on while already touching the player, before it had a value.
	// That overlap event fired with 0 XP and was ignored, so check again now.
	TArray<AActor*> Overlapping;
	BoxCollision->GetOverlappingActors(Overlapping, AUC_Player::StaticClass());
	TArray<UPrimitiveComponent*> OverlappingComp;
	BoxCollision->GetOverlappingComponents(OverlappingComp);
	//Added a component check so the EXP pick up only happens on contacting the player's capsule
	//however this section lacks that part so I added getting the Uprimitive needed for the new CanPickUp Check
	//it pairs the actor and component before doing the check.
	for (AActor* Actor:  Overlapping)
	{
		for (UPrimitiveComponent* Comp : OverlappingComp) {
			AActor* CompActor = Comp->GetOwner();
			if (CompActor) {
				if (Actor == CompActor) {
					if (CanPickUpCompCheck(Actor, Comp))
					{
						HandlePickUp();
						PostPickUp();
						return;
					}
				}
			}
		}
	}
	//when to become the next tier of exp
	if (Amount >= Tier3Requirment) {
		Sprite->SetVisibility(false);
		Sprite2->SetVisibility(false);
		Sprite3->SetVisibility(true);
	}
	else if (Amount >= Tier2Requirment) {
		Sprite->SetVisibility(false);
		Sprite2->SetVisibility(true);
		Sprite3->SetVisibility(false);
	}

	EXPValue = Amount;

}

void AEXP_PickUp::OnPoolAcquire_Implementation()
{
	// Fresh gem: no value until the enemy sets one, no remembered player
	EXPValue = 0;
	SavedPlayer = nullptr;
	GI = Cast<UUGI_SUGameInstance>(GetGameInstance());
	if (GI) {
		if (this) {
			GI->EXPActors.Add(this);
			MergeTime();
		}
	}
}

void AEXP_PickUp::OnPoolRelease_Implementation()
{

	SavedPlayer = nullptr;
	GI = nullptr;
}

// Called when the game starts or when spawned
void AEXP_PickUp::BeginPlay()
{
	Super::BeginPlay();
	//for testin
	GI = Cast<UUGI_SUGameInstance>(GetGameInstance());
	RootComponent->SetMobility(EComponentMobility::Movable);

	if (GI) {
		if (this) {
			GI->EXPActors.Add(this);
			MergeTime();
		}
	}

}

bool AEXP_PickUp::CanPickUpCompCheck(AActor* OtherActor, UPrimitiveComponent* OverlapComponent)
{
	bool result = false;
	AUC_Player* ThePlayer = Cast<AUC_Player>(OtherActor);
	if (ThePlayer) {
		UCapsuleComponent* OlC = Cast<UCapsuleComponent>(OverlapComponent);
		if (OlC) {
			if (OlC == ThePlayer->GetCapsuleComponent()) {
				result = true;
				SavedPlayer = ThePlayer;
			}
		}
	}
	return result;
}


void AEXP_PickUp::PostPickUp()
{
	DisableEXPTick();
	Sprite->SetVisibility(true);
	Sprite2->SetVisibility(false);
	Sprite3->SetVisibility(false); //reset tier look for next one as well!
	if (GI) {
		GI->EXPActors.Remove(this);
	}
	if (MergeMode == true) {
		GI->calledMerge = false;
	}
	MergeMode = false;
	DisableEXPTick();
	AttractedActors.Empty();
	GetWorldTimerManager().ClearTimer(TimerHandle);
	RootComponent->SetMobility(EComponentMobility::Movable);
	if (UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
	{
		Pool->Release(this);
	}
	else Destroy();
}

void AEXP_PickUp::HandlePickUp()
{
	if (SavedPlayer) {
		SavedPlayer->GetPlayerStatsComp()->UpdateExpANDLevel(EXPValue);
		SavedPlayer->AttractedActors.Remove(this);
	}
}

void AEXP_PickUp::HandleOverLap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult)
{
	if (EXPValue <= 0 || OtherActor == this)
	{
		return; // not ready yet; SetExpValue will collect it if the player is still here
	}
	if (this->MergeMode == true && !this->AttractedActors.IsEmpty()) {
		AEXP_PickUp* EXP = Cast<AEXP_PickUp>(OtherActor);
		if (EXP) {
			AttractedActors.Remove(OtherActor);
			Merge(this, EXP);
		}
		if (AttractedActors.IsEmpty()) {
			MergeMode = false;
			DisableEXPTick();
		}
	}
	if (CanPickUpCompCheck(OtherActor, OtherComp)) {
		HandlePickUp();
		PostPickUp();
	}
}

int AEXP_PickUp::GetEXPValue()
{
	return EXPValue;
}

void AEXP_PickUp::MergeTime()
{
	if (GetWorld()) {
		GetWorldTimerManager().SetTimer(TimerHandle, GI, &UUGI_SUGameInstance::EnableMerge, MergeTimer, false);
	}
}

void AEXP_PickUp::ForceMergeTime()
{
	if (GetWorld()) {
		GetWorldTimerManager().SetTimer(ForceMergeTimerHandle, this, &AEXP_PickUp::ForceMerge, FlyFor, false);
	}
}



void AEXP_PickUp::MergetoSelf()
{
	if (GI) {
		for (int i = 0; i < GI->EXPActors.Num();i++) {
			if (GI->EXPActors[i] != this && !GI->PlayerItself->AttractedActors.Contains(GI->EXPActors[i])) {
				AttractedActors.Add(GI->EXPActors[i]);
			}
		}
		EnableEXPTick();
	}
}

void AEXP_PickUp::FlySpeedSet()
{
	EXPFlySpeed = EXPMinFlySpeed;
}

UBoxComponent* AEXP_PickUp::GetBoxComponent()
{
	return BoxCollision;
}

void AEXP_PickUp::EnableEXPTick()
{

	if (GI) {
		bool check = false;
		for (int i = 0; i < GI->EXPActors.Num();i++) {
			AEXP_PickUp* EXP = Cast<AEXP_PickUp>(GI->EXPActors[i]);
			if (EXP->MergeMode) {
				check = true;
			}
		}
		if (check== false) {
			SetActorTickEnabled(true);
			MergeMode = true;
		}
	}

}

void AEXP_PickUp::DisableEXPTick()
{
	SetActorTickEnabled(false);
	FlySpeedSet();
	if (GI) {
		GI->calledMerge = false;
	}

}


// Called every frame
void AEXP_PickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	for (int32 i = 0; i < AttractedActors.Num(); i++) {
		AActor* Actor = AttractedActors[i].Get();
		if (Actor) {
			FVector Direction = (this->GetActorLocation() - Actor->GetActorLocation()).GetSafeNormal();
			FVector Offset = Direction * EXPFlySpeed * DeltaTime;
			Actor->AddActorLocalOffset(Offset, false);
		}
	}
	EXPFlySpeed = FMath::Clamp(EXPFlySpeed + (EXPFlySpeedRamp * DeltaTime), EXPMinFlySpeed, EXPMaxFlySpeed);
}
void  AEXP_PickUp::Merge(AEXP_PickUp* Mergeinto, AEXP_PickUp* Mergewith) {
	if (Mergewith) {
		if (Mergewith->GetEXPValue() > 0) {
			Mergeinto->SetExpValue(Mergeinto->GetEXPValue() + Mergewith->GetEXPValue());
			Mergewith->PostPickUp();
		}
	}
}

void AEXP_PickUp::ForceMerge()
{
	for (int32 i = 0; i < AttractedActors.Num(); i++) {
		AEXP_PickUp* EXP = Cast<AEXP_PickUp>(AttractedActors[i].Get());
		if (EXP) {
			AttractedActors.Remove(AttractedActors[i].Get());
			Merge(this, EXP);
		}
		if (AttractedActors.IsEmpty()) {
			MergeMode = false;
			DisableEXPTick();
		}
	}
}

