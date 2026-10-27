// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Animation/PlayerAnimInstance.h"
//#include "KismetAnimationLibrary.h"

void UPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();

	if (Pawn)
	{
		//isValid
		Velocity = Pawn->GetVelocity().Length();
		//Direction = UKismetAnimationLibrary::CalculateDirection(Pawn->GetVelocity(), Pawn->GetActorRotation());
	}
	else
	{
		//notValid

		//Preview Window
		PreviewWindowUpdate();
	}
}

void UPlayerAnimInstance::PreviewWindowUpdate_Implementation()
{
	if (DebugFire) {
		DebugFire = false;
		FireAnimation();
	}
}

UPlayerAnimInstance::UPlayerAnimInstance()
{
	//FireAsset = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Script/Engine.AnimSequence'/Game/END_Starter/Mannequin/Animations/A_Fire_Ironsights.A_Fire_Ironsights'"));
	//ReloadAsset = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Script/Engine.AnimSequence'/Game/END_Starter/Mannequin/Animations/A_Reload_Ironsights.A_Reload_Ironsights'"));
	//HitAsset = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Script/Engine.AnimSequence'/Game/END_Starter/Mannequin/Animations/A_Hit_Ironsights.A_Hit_Ironsights'"));
	//UAnimSequenceBase* Asset3 = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Script/Engine.AnimSequence'/Game/END_Starter/Mannequin/Animations/A_Death1_Ironsights.A_Death1_Ironsights'"));
	//DeadAssets.Add(Asset3);
	//UAnimSequenceBase* Asset4 = LoadObject<UAnimSequenceBase>(nullptr, TEXT("/Script/Engine.AnimSequence'/Game/END_Starter/Mannequin/Animations/A_Death2_Ironsights.A_Death2_Ironsights'"));
	//DeadAssets.Add(Asset4);
	ActionSlotName = "Action";
}


void UPlayerAnimInstance::FireAnimation()
{
	UAnimInstance::PlaySlotAnimationAsDynamicMontage(FireAsset, ActionSlotName);
}

void UPlayerAnimInstance::ReloadAnimation()
{
	UAnimInstance::PlaySlotAnimationAsDynamicMontage(ReloadAsset, ActionSlotName);
}
void UPlayerAnimInstance::HitAnimation()
{
	UAnimInstance::PlaySlotAnimationAsDynamicMontage(HitAsset, ActionSlotName);
}

void UPlayerAnimInstance::DeathEnded()
{
	//CDE.Broadcast();
}

void UPlayerAnimInstance::DeadAnimation_Implementation()
{
	int32 RanInd = FMath::RandRange(0, DeadAssets.Num() - 1);
	if (RanInd < 0 || RanInd > DeadAssets.Num() - 1) {
		RanInd = 0;
	}
	DeadAsset = DeadAssets[RanInd];

	FTimerDynamicDelegate DeleteDelegate;
	FTimerManagerTimerParameters param;
	param.bLoop = false;
	DeleteDelegate.BindDynamic(this, &UPlayerAnimInstance::DeathEnded);
	GetWorld()->GetTimerManager().SetTimer(DeleteTimerHandle, DeleteDelegate, DeadAsset->GetPlayLength(), param);
	//UAnimInstance::PlaySlotAnimationAsDynamicMontage(DeadAsset, ActionSlotName);
}