// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DebugOverlay.h"

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

void UDebugOverlay::SetTitle(const FName& String)
{
	Title->SetText(FText::FromName(String));
}