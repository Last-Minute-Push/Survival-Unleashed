// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DebugOverlayDatabase.h"
#include "DebugOverlaySubsystem.generated.h"

class UDebugOverlayDatabase;

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UDebugOverlaySubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
	bool bPerFrame;
	TObjectPtr<UDebugOverlayDatabase> DataOrigin;
	TMap<TTuple<FName, FName, FName>, FDebugValueData> DataFrontend;

	TObjectPtr<UDebugOverlay> CurrentOverlay;

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	void InitializeDebugOverlay(const FString& Overlay = "", const bool bUpdatePerFrame = true);
};