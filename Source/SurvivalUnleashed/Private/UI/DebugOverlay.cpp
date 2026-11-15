// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DebugOverlay.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/CanvasPanelSlot.h"

DEFINE_LOG_CATEGORY_STATIC(LogDebugOverlay, Log, All)

void UDebugOverlay::AddToContainer(TWeakObjectPtr<UUserWidget> Widget)
{
	if (!Widget.IsValid())
	{
		UE_LOG(LogDebugOverlay, Warning, TEXT("AddToContainer Widget is not valid!"));
		return;
	}

	Container->AddChild(Widget.Get());
}

void UDebugOverlay::SetZOrder(int32 ZOrder)
{
	if (IsInViewport()) RemoveFromParent();
	AddToViewport(ZOrder);
}

void UDebugOverlay::SetTitle(const FName& Name)
{
	Title->SetText(FText::FromName(Name));
}

void UDebugOverlay::SetBackgroundColor(FLinearColor Color)
{
	Background->SetBrushColor(Color);
}