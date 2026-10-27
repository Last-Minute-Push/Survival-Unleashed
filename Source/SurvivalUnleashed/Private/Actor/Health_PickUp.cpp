// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Health_PickUp.h"
#include "Actor/UC_Player.h"
#include "Components/BoxComponent.h"
#include "PaperSpriteComponent.h"
#include "Components/CapsuleComponent.h"
#include "Framework/ObjectPoolSubsystem.h"

// Sets default values
AHealth_PickUp::AHealth_PickUp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	BoxCollision = CreateDefaultSubobject<UBoxComponent>("BoxCollision");
	RootComponent = BoxCollision;
	Sprite = CreateDefaultSubobject<UPaperSpriteComponent>("Sprite");
	Sprite->SetupAttachment(BoxCollision);
	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AHealth_PickUp::HandleOverLap);
}

// Called when the game starts or when spawned
void AHealth_PickUp::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHealth_PickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AHealth_PickUp::SetHealValue(int Amount)
{
	TArray<AActor*> Overlapping;
	BoxCollision->GetOverlappingActors(Overlapping, AUC_Player::StaticClass());
	TArray<UPrimitiveComponent*> OverlappingComp;
	BoxCollision->GetOverlappingComponents(OverlappingComp);

	for (AActor* Actor : Overlapping)
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
	Heal = Amount;
}

void AHealth_PickUp::OnPoolAcquire_Implementation()
{
	SavedPlayer = nullptr;
}

void AHealth_PickUp::OnPoolRelease_Implementation()
{
	SavedPlayer = nullptr;
}
bool AHealth_PickUp::CanPickUpCompCheck(AActor* OtherActor, UPrimitiveComponent* OverlapComponent) {
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
void AHealth_PickUp::PostPickUp() {
	if (UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
	{
		Pool->Release(this);
	}
	else Destroy();
}
void AHealth_PickUp::HandlePickUp() {
	if (SavedPlayer) {
		if (SavedPlayer->GetPlayerStatsComp()->GetHealAmount() >0) {
			UGameplayStatics::ApplyDamage(SavedPlayer, -Heal * SavedPlayer->GetPlayerStatsComp()->GetHealAmount(), nullptr, this, nullptr);
		}
		else {
			UGameplayStatics::ApplyDamage(SavedPlayer, -Heal, nullptr, this, nullptr);
		}
	}
}

void AHealth_PickUp::HandleOverLap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult) {
	if (Heal <= 0)
	{
		return;
	}

	if (CanPickUpCompCheck(OtherActor, OtherComp)) {
		HandlePickUp();
		PostPickUp();
	}
}