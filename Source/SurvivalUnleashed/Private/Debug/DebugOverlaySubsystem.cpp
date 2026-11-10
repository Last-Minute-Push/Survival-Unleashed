// Fill out your copyright notice in the Description page of Project Settings.


#include "Debug/DebugOverlaySubsystem.h"
#include "Framework/UGI_SUGameInstance.h"
#include "UI/DebugOverlay.h"
#include "UI/DebugValue.h"

DEFINE_LOG_CATEGORY_STATIC(LogDebugOverlaySubsystem, Log, All);

void UDebugOverlaySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(InWorld.GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogDebugOverlaySubsystem, Error, TEXT("Failed to retireve GameInstance!"));
		return;
	}
	
	UDebugOverlayDatabase* DB = GI->GetOverlayDatabase();
	if (!DB)
	{
		UE_LOG(LogDebugOverlaySubsystem, Error, TEXT("Failed to retrieve Debug Overlay Database!"));
		return;
	}

	DataOrigin = DB;

	for (FDebugOverlayData& Overlay : DataOrigin->Database)
		for (FDebugCategoryData& Category : Overlay.Categories)
			for (auto Value = Category.DebugValues.begin(); Value != Category.DebugValues.end(); ++Value)
			{
				DataFrontend.Add({Overlay.Title, Category.Title, Value->Key}, Value->Value);
			}
}

void UDebugOverlaySubsystem::InitializeDebugOverlay(const FString& OverlayName, const bool bUpdatePerFrame/*= true*/)
{
	if (!DataOrigin || DataFrontend.IsEmpty())
	{
		UE_LOG(LogDebugOverlaySubsystem, Error, TEXT("There is no debug overlay database; debug ovelay will not display!"));
		return;
	}

	bool bNameFound = false;
	bPerFrame = bUpdatePerFrame;

	for (FDebugOverlayData& Overlay : DataOrigin->Database)
	{
		Overlay.Widget = CreateWidget<UDebugOverlay>(this);
		if (!Overlay.Widget) continue;

		Overlay.Widget->SetTitle(Overlay.Title);

		for (FDebugCategoryData& Category : Overlay.Categories)
		{
			Category.Widget = CreateWidget<UDebugOverlay>(this);
			if (!Category.Widget) continue;

			Category.Widget->SetTitle(Category.Title);

			for (auto Value = Category.DebugValues.begin(); Value != Category.DebugValues.end(); ++Value)
			{
				Value->Value.Widget = CreateWidget<UDebugValue>(this);
				if (!Value->Value.Widget) continue;

				Value->Value.Widget->SetName(Value->Key);
				Category.Widget->AddToContainer(Value->Value.Widget);
			}

			Overlay.Widget->AddToContainer(Category.Widget);
		}

		Overlay.Widget->AddToViewport();

		if (Overlay.Title == OverlayName)
		{
			bNameFound = true;
			Overlay.Widget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Overlay.Widget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (!bNameFound)
	{
		DataOrigin->Database[0].Widget->SetVisibility(ESlateVisibility::Visible);
	}
}