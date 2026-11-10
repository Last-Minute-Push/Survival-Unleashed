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

	bool MergeMode = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite) //for testing
	int EXPValue = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) 
	int Tier2Requirment = 100; //what the exp amount needed for tier 2 exp look
	UPROPERTY(EditAnywhere, BlueprintReadWrite) 
	int Tier3Requirment = 250; //what the exp amount needed for tier 3 exp look

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EXPMinFlySpeed = 400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EXPMaxFlySpeed = 1200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EXPFlySpeedRamp = 90.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EXPFlySpeed = 0;
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> AttractedActors;
	void SetExpValue(int Amount); //for enemies to set the expvalue


	void Merge(AEXP_PickUp* Mergeinto, AEXP_PickUp* Mergewith);
	UFUNCTION()
	void EnableEXPTick();
	UFUNCTION()
	void DisableEXPTick();

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


	void FlySpeedSet();

	UBoxComponent* GetBoxComponent();
};
