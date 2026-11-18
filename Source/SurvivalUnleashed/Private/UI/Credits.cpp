// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Credits.h"
#include "UI/ButtonBase.h"
#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"

void UCredits::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BackButton->CodeButtonClick.AddDynamic(this, &UCredits::BackButtonPressed);
	PlayAnimation(CreditsAnimation);
	this->OnCreditsFinish.BindDynamic(this, &UCredits::BackButtonPressed);
	this->UUserWidget::BindToAnimationFinished(CreditsAnimation, OnCreditsFinish);
	PlayAnimation(CreditsAnimation);
}

void UCredits::BackButtonPressed()
{
	RemoveFromParent();
	OnCreditsClose.Broadcast();
}
