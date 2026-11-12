// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SplashScreen.h"
#include "Framework/UGI_SUGameInstance.h"
#include "Components/WidgetInteractionComponent.h"
#include "Animation/WidgetAnimation.h"
#include "Animation/WidgetAnimationPlayCallbackProxy.h"
#include "EditorViewportClient.h"
#include "Input/Reply.h"
#include "SurvivalUnleashed/SurvivalUnleashed.h"

DEFINE_LOG_CATEGORY_STATIC(LogSplashScreen, Log, All);

void USplashScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	this->bIsFocusable = true;
	
	this->OnSplashFinish.BindDynamic(this, &USplashScreen::SkippedIntro);
	this->UUserWidget::BindToAnimationFinished(SplashIntroAnimation, OnSplashFinish);
	PlayAnimation(SplashIntroAnimation);
}

FReply USplashScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	Super::NativeOnKeyDown(InGeometry, InKeyEvent);

	SkippedIntro();

	return  OnKeyDown(InGeometry, InKeyEvent).NativeReply;
}



void USplashScreen::SkippedIntro()
{
	if (UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance()))
	{
		GI->QuitToMenu();
	}
	UE_LOG(LogSplashScreen, Error, TEXT("SkippedIntro Worked and is ready for implimentation!!!"));
}
