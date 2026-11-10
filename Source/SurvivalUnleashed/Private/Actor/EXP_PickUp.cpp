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
	PrimaryActorTick.bCanEverTick = false;
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
	Radius = CreateDefaultSubobject<USphereComponent>("PickUpRadius");
	Radius->SetupAttachment(BoxCollision);
	Radius->SetSphereRadius(EXPRadius);
	Radius->Deactivate();
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
	OneTime = true; //let it enable merge again on next remake
}

void AEXP_PickUp::OnPoolRelease_Implementation()
{
	SavedPlayer = nullptr;
	OneTime = true; //let it enable merge again on next remake
}

// Called when the game starts or when spawned
void AEXP_PickUp::BeginPlay()
{
	Super::BeginPlay();
	if (OneTime == true) {
		MergeClock();
		OneTime = false;
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
	Sprite->SetVisibility(true);
	Sprite2->SetVisibility(false);
	Sprite3->SetVisibility(false); //reset tier look for next one as well!

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
	if (EXPValue <= 0)
	{
		return; // not ready yet; SetExpValue will collect it if the player is still here
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

void AEXP_PickUp::MergeClock()// starts the merge loop
{
	
	if (GetWorld()) {
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AEXP_PickUp::EnableMerge, MergeTimer, false);
	}
	else {
		OneTime = true; //try again
	}
}

void AEXP_PickUp::EnableMerge() //enable merge for a second
{
	Radius->Activate(true);
	Radius->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DontRemove = true;
	TArray<AActor*> OverlappingActors;
	Radius->GetOverlappingActors(OverlappingActors, AEXP_PickUp::StaticClass());
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor)
		{
			if (Actor != this) {
				AEXP_PickUp* EXP_PickUp = Cast<AEXP_PickUp>(Actor);
				if (EXP_PickUp) {
					if (EXP_PickUp->GetEXPValue() > 0) {
						SetExpValue(GetEXPValue() + EXP_PickUp->GetEXPValue());
						if (UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
						{
							Pool->Release(EXP_PickUp);
						}
						else Destroy();
					}
				}
				else {
					return;
				}
			}
		}
	}
	if (GetWorld()) {
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AEXP_PickUp::DisableMerge, DisableMergeTimer, false);
	}
}

void AEXP_PickUp::DisableMerge() //makes this merge thing loop until it gets picked up;
{
	Radius->Deactivate();
	if (GetWorld()) {
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AEXP_PickUp::EnableMerge, MergeTimer, false);
	}
}

// Called every frame
void AEXP_PickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


