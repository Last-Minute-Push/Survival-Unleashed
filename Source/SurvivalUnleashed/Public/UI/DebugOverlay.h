// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DebugOverlay.generated.h"

class UTextBlock;
class UVerticalBox;
/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UDebugOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Title;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UVerticalBox> Container;
};
