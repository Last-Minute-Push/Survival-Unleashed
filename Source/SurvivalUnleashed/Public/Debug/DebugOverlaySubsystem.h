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
	float PerSecondTimer = 0.f;
	int32 FPS, CurrentOverlayIdx = 0;
	FDebugOverlayData* CurrentOverlay;
	TObjectPtr<UDebugOverlayDatabase> OverlayData;
	TMap<TTuple<FName, FName, FName>, FDebugValueData*> OverlayFrontend;
	TObjectPtr<APlayerController> PlayerController;

	void AttachDefaultValues();
	void SetValue(FDebugValueData& Value);
	FDebugValueData* FindKey(const FName& OverlayName, const FName& CategoryName, const FName& VariableName);
	UFUNCTION() inline int32 GetFPS() const { return FPS; }
	UFUNCTION() void HandleDamagePerSecond(AEnemyBase* Enemy, float Damage, AActor* DamageCauser);

protected:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual bool IsTickable() const override { return bTick; }
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(UDebugOverlaySubsystem, STATGROUP_Tickables); }

public:
	EDebugOverlayInit InitializeDebugOverlay(const FString& Overlay = "");

	void AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateBool GetValue, FOnOverlayResetValue ResetValue = nullptr);
	void AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateInt32 GetValue, FOnOverlayResetValue ResetValue = nullptr);
	void AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateFloat GetValue, FOnOverlayResetValue ResetValue = nullptr);
	void AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateVector2 GetValue, FOnOverlayResetValue ResetValue = nullptr);
	void AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateVector3 GetValue, FOnOverlayResetValue ResetValue = nullptr);
	void AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateVector4 GetValue, FOnOverlayResetValue ResetValue = nullptr);
	void AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateString GetValue, FOnOverlayResetValue ResetValue = nullptr);

	void ChangeOverlay(const int32 NewIdx);
	void GetOverlayNames(TArray<FString>& OverlayNames);

	inline int32 GetOverlayIndex() const { return CurrentOverlayIdx; }
};