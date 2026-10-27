// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Framework/Poolable.h"
#include "Health_PickUp.generated.h"

UCLASS()
class SURVIVALUNLEASHED_API AHealth_PickUp : public AActor, public IPoolable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHealth_PickUp();
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBoxComponent* BoxCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UPaperSpriteComponent* Sprite;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) //for testing
		int Heal = 0;
	void SetHealValue(int Amount); //for it to set heal amount;

	// IPoolable
	virtual void OnPoolAcquire_Implementation() override;
	virtual void OnPoolRelease_Implementation() override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	class AUC_Player* SavedPlayer;
	bool CanPickUpCompCheck(AActor* OtherActor, UPrimitiveComponent* OverlapComponent);
	void PostPickUp();
	void HandlePickUp();
	UFUNCTION()
	void HandleOverLap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult);


};
