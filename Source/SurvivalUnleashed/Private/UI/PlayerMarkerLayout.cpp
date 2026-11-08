// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMarkerLayout.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlayerMarkerLayout, Log, All);
void UPlayerMarkerLayout::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPlayerMarkerLayout::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (GetOwningPlayerPawn()) {
		UpdateMarkerPosition(GetOwningPlayerPawn()->GetActorLocation());
	}
}

void UPlayerMarkerLayout::UpdateMarkerPosition(FVector location)
{
	if (PlayerMarker)
	{
	
		UCanvasPanelSlot* CanvasSlot = PlayerMarkerCanvas->AddChildToCanvas(PlayerMarker);
		FVector2D WidgetPos;
		
		if (CanvasSlot)
		{
			if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(), location, WidgetPos, false))
			{
				
				WidgetPos.Y = WidgetPos.Y - 50;
				CanvasSlot->SetSize({ 50,50 });
				CanvasSlot->SetAlignment({ 0.5,0.5 });
				CanvasSlot->SetPosition(WidgetPos);
			}
			else
			{
				UE_LOG(LogPlayerMarkerLayout, Error, TEXT("CanvasSlot projection set up gone wrong!"));
			}
		}
		else
		{
			UE_LOG(LogPlayerMarkerLayout, Error, TEXT("CanvasSlot was a failure, different approach must be made!"));
		}
	}
	
}
