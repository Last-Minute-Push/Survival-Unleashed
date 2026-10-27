// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/EXP_PickUp.h"
#include "Actor/UC_Player.h"
#include "Components/BoxComponent.h"
#include "PaperSpriteComponent.h"
#include "Components/CapsuleComponent.h"
#include "Framework/ObjectPoolSubsystem.h"


// Sets default values
AEXP_PickUp::AEXP_PickUp()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	BoxCollision = CreateDefaultSubobject<UBoxComponent>("BoxCollision");
	RootComponent = BoxCollision;
	Sprite = CreateDefaultSubobject<UPaperSpriteComponent>("Sprite");
	Sprite->SetupAttachment(BoxCollision);
	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AEXP_PickUp::HandleOverLap);
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

	EXPValue = Amount;
}

void AEXP_PickUp::OnPoolAcquire_Implementation()
{
	// Fresh gem: no value until the enemy sets one, no remembered player
	EXPValue = 0;
	SavedPlayer = nullptr;
}

void AEXP_PickUp::OnPoolRelease_Implementation()
{
	SavedPlayer = nullptr;
}

// Called when the game starts or when spawned
void AEXP_PickUp::BeginPlay()
{
	Super::BeginPlay();

}

bool AEXP_PickUp::CanPickUp(AActor* OtherActor)
{
	bool result = false;
	AUC_Player* ThePlayer = Cast<AUC_Player>(OtherActor);
	if (ThePlayer) {
		result = true;
		SavedPlayer = ThePlayer;
	}
	return result;
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

// Called every frame
void AEXP_PickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

