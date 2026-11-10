// Fill out your copyright notice in the Description page of Project Settings.


#include "Debug/DebugOverlaySubsystem.h"
#include "Framework/UGI_SUGameInstance.h"

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
				DataFrontend.Add({Overlay.Title, Category.Name, Value->Key}, Value->Value);
			}
}

void UDebugOverlaySubsystem::InitializeDebugOverlay(const FString& OverlayName = "", const bool bUpdatePerFrame/*= true*/)
{
	if (!DataOrigin || DataFrontend.IsEmpty())
	{
		UE_LOG(LogDebugOverlaySubsystem, Error, TEXT("There is no debug overlay database; debug ovelay will not display!"));
		return;
	}

	bPerFrame = bUpdatePerFrame;

	for (FDebugOverlayData& Overlay : DataOrigin->Database)
	{
		// Start the initialization of all overlays and
		// display OverlayName only or the first overlay in the TArray
	}
}