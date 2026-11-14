// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/HUDBase.h"
#include "DebugOverlay.generated.h"

class UBorder;
class UTextBlock;
class UVerticalBox;

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UDebugOverlay : public UHUDBase
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Title;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UBorder> Background;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UVerticalBox> Container;

public:
	void AddToContainer(TWeakObjectPtr<UUserWidget> Widget);
	void SetBackgroundColor(FLinearColor Color);
	void SetZOrder(int32 ZOrder);

	inline void SetTitle(const FName& Name);
};