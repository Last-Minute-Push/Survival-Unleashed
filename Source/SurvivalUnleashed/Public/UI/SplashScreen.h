// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SplashScreen.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API USplashScreen : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeOnInitialized() override;

protected:

	
};
