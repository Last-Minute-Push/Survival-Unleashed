// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Magnet_PickUp.h"
#include "Actor/UC_Player.h"
#include "Components/BoxComponent.h"
#include "PaperSpriteComponent.h"
#include "Components/CapsuleComponent.h"
#include "Framework/ObjectPoolSubsystem.h"

// Sets default values
AMagnet_PickUp::AMagnet_PickUp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	BoxCollision = CreateDefaultSubobject<UBoxComponent>("BoxCollision");
	RootComponent = BoxCollision;
	Sprite = CreateDefaultSubobject<UPaperSpriteComponent>("Sprite");
	Sprite->SetupAttachment(BoxCollision);
	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AMagnet_PickUp::HandleOverLap);
}

void AMagnet_PickUp::OnPoolAcquire_Implementation()
{
	SavedPlayer = nullptr;
}

void AMagnet_PickUp::OnPoolRelease_Implementation()
{
	SavedPlayer = nullptr;
}

// Called when the game starts or when spawned
void AMagnet_PickUp::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMagnet_PickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AMagnet_PickUp::CanPickUpCompCheck(AActor* OtherActor, UPrimitiveComponent* OverlapComponent)
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

void AMagnet_PickUp::PostPickUp()
{
	if (UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
	{
		Pool->Release(this);
	}
	else Destroy();
}

void AMagnet_PickUp::HandlePickUp()
{
	if (SavedPlayer) {
		SavedPlayer->MagnetEnable();
		SavedPlayer->AttractedActors.Remove(this);
	}
}

void AMagnet_PickUp::HandleOverLap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult)
{

	if (CanPickUpCompCheck(OtherActor, OtherComp)) {
		HandlePickUp();
		PostPickUp();
	}
}
