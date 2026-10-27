// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

/**
 * 
 */
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FReloadHUD);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCallReloadEnded);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCallDeathEnded);


UCLASS()
class SURVIVALUNLEASHED_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds)override;
	UPlayerAnimInstance();
	UFUNCTION(BlueprintNativeEvent)
	void PreviewWindowUpdate();
	UFUNCTION(BlueprintCallable)
	void FireAnimation();
	UFUNCTION(BlueprintCallable)
	void ReloadAnimation();
	UFUNCTION(BlueprintCallable)
	void HitAnimation();
	UFUNCTION(BlueprintCallable)
	void DeathEnded();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DeadAnimation();
	//UPROPERTY(VisibleAnywhere, BlueprintAssignable, BlueprintCallable, Category = "ReloadHud")
	//FReloadHUD RHUD;
	//UPROPERTY(VisibleAnywhere, BlueprintAssignable, BlueprintCallable, Category = "ReloadEnd")
	//FCallReloadEnded CRE;
	//UPROPERTY(VisibleAnywhere, BlueprintAssignable, BlueprintCallable, Category = "ReloadEnd")
	//FCallDeathEnded CDE;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Default)
	float Velocity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Default)
	float Direction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Default)
	UAnimSequenceBase* FireAsset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Default)
	UAnimSequenceBase* HitAsset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Default)
	TArray<UAnimSequenceBase*> DeadAssets;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	UAnimSequenceBase* DeadAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	UAnimSequenceBase* ReloadAsset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Default)
	FName ActionSlotName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Default)
	bool DebugFire;
	FTimerHandle DeleteTimerHandle;
};
