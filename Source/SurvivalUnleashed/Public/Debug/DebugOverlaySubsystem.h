// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DebugOverlayDatabase.h"
#include "DebugOverlaySubsystem.generated.h"

class AUC_Player;
class UDebugOverlayDatabase;

enum EDebugOverlayInit
{
	ESuccess = 0,
	ENoneExist,
	EFailed
};

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UDebugOverlaySubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
	bool bPerFrame, bTick = false;
	FDebugOverlayData* CurrentOverlay;
	TObjectPtr<UDebugOverlayDatabase> OverlayData;
	TMap<TTuple<FName, FName, FName>, FDebugValueData*> OverlayFrontend;
	TObjectPtr<APlayerController> PlayerController;

	void SetValue(FDebugValueData& Value);
	FDebugValueData* FindKey(const FName& OverlayName, const FName& CategoryName, const FName& VariableName);

protected:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual bool IsTickable() const override { return bTick; }
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(UDebugOverlaySubsystem, STATGROUP_Tickables); }

public:
	EDebugOverlayInit InitializeDebugOverlay(const FString& Overlay = "", const bool bUpdatePerFrame = true);

	void AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateBool GetBool);
	void AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateInt32 GetInt32);
	void AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateFloat GetFloat);
	void AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateVector2 GetVector2);
	void AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateVector3 GetVector3);
	void AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateVector4 GetVector4);
	void AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateString GetString);

	void GetOverlayNames(TArray<FString>& OverlayNames);
};