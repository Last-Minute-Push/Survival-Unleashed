// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "PlayerMarkerLayout.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UPlayerMarkerLayout : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UCanvasPanel* PlayerMarkerCanvas;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UImage* PlayerMarker;

	UPROPERTY(EditDefaultsOnly) double WidgetAmountOffset;

public:
	void UpdateMarkerPosition(FVector location);
	
};
