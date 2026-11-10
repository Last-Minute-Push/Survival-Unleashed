// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Framework/Poolable.h"
#include "EXP_PickUp.generated.h"

UCLASS()
class SURVIVALUNLEASHED_API AEXP_PickUp : public AActor, public IPoolable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEXP_PickUp();
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBoxComponent* BoxCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UPaperSpriteComponent* Sprite;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UPaperSpriteComponent* Sprite2; //holds tier2 look
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UPaperSpriteComponent* Sprite3; //holds tier3 look

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* Radius;


	UPROPERTY(EditAnywhere, BlueprintReadWrite) //for testing
	int EXPValue = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) 
	int EXPRadius = 400; //how far an exp can pick up other exp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) 
	int Tier2Requirment = 100; //what the exp amount needed for tier 2 exp look
	UPROPERTY(EditAnywhere, BlueprintReadWrite) 
	int Tier3Requirment = 250; //what the exp amount needed for tier 3 exp look
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MergeTimer = 10.0f; //how long to wait until merge
	UPROPERTY(EditAnywhere, BlueprintReadWrite) 
	float DisableMergeTimer = 5.0f; //how long to wait to stop allow merging.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTimerHandle TimerHandle;


	void SetExpValue(int Amount); //for enemies to set the expvalue

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
	int GetEXPValue();
	UFUNCTION()
	void HandleEXPOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void MergeClock();
	UFUNCTION()
	void EnableMerge();
	UFUNCTION()
	void DisableMerge();
	bool OneTime = true;
	bool DontRemove = false;
};
