// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/HUDBase.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "DebugOverlay.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UDebugOverlay : public UHUDBase
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Title;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UVerticalBox> Container;

public:
	void AddToContainer(TWeakObjectPtr<UUserWidget> Widget);
	void SetTitle(const FName& String);
};