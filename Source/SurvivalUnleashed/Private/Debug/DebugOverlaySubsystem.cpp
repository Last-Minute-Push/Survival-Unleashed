// Fill out your copyright notice in the Description page of Project Settings.


#include "Debug/DebugOverlaySubsystem.h"
#include "Framework/UGI_SUGameInstance.h"
#include "UI/DebugOverlay.h"
#include "UI/DebugValue.h"
#include "Actor/UC_Player.h"
#include "GameFramework/CheatManagerDefines.h"

DEFINE_LOG_CATEGORY_STATIC(LogDebugOverlaySubsystem, Log, All);

void UDebugOverlaySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

#if UE_WITH_CHEAT_MANAGER
	UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(InWorld.GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogDebugOverlaySubsystem, Error, TEXT("Failed to retireve GameInstance!"));
		return;
	}
	
	APlayerController* PC = Cast<APlayerController>(GI->GetFirstLocalPlayerController(GetWorld()));
	if (!PC)
	{
		UE_LOG(LogDebugOverlaySubsystem, Error, TEXT("Failed to retrieve Player!"));
		return;
	}
	PlayerController = PC;

	UDebugOverlayDatabase* DB = GI->GetOverlayDatabase();
	if (!DB)
	{
		UE_LOG(LogDebugOverlaySubsystem, Error, TEXT("No debug overlay is set in the game instance!"));
		return;
	}
	OverlayData = DB;

	for (FDebugOverlayData& Overlay : OverlayData->Database)
		for (FDebugCategoryData& Category : Overlay.Categories)
			for (auto Value = Category.DebugValues.begin(); Value != Category.DebugValues.end(); ++Value)
				OverlayFrontend.Add({Overlay.Title, Category.Title, Value->Key}, &Value->Value);

#endif
}

void UDebugOverlaySubsystem::Tick(float DeltaTime)
{
#if UE_WITH_CHEAT_MANAGER
	checkf(IsInitialized(), TEXT("Ticking should have been disabled for an uninitialized subsystem!"));

	PerSecondTimer += DeltaTime;
	FPS = FMath::TruncToInt32<float>(1.f / DeltaTime);

	for (FDebugCategoryData& Category : CurrentOverlay->Categories)
		for (auto Value = Category.DebugValues.begin(); Value != Category.DebugValues.end(); ++Value)
		{
			if (!Value->Value.bUpdatePerSecond)
			{
				SetValue(Value->Value);
			}
			else if (PerSecondTimer >= 1.f)
			{
				PerSecondTimer = 0;
				SetValue(Value->Value);
			}

		}
#endif
}

EDebugOverlayInit UDebugOverlaySubsystem::InitializeDebugOverlay(const FString& OverlayName)
{
#if UE_WITH_CHEAT_MANAGER
	if (!IsValid(OverlayData) || OverlayFrontend.IsEmpty() || !IsValid(PlayerController) ||
		!IsValid(OverlayData->OverlayClass) || !IsValid(OverlayData->CategoryClass) || !IsValid(OverlayData->ValueClass))
	{
		UE_LOG(LogDebugOverlaySubsystem, Error, TEXT("There is no debug overlay database or invalid initialization; debug ovelay will not display!"));
		return EDebugOverlayInit::EFailed;
	}

	bool bOverlayNameFound = false;

	AttachDefaultValues();

	for (int32 I = 0; I < OverlayData->Database.Num(); ++I)
	{
		FDebugOverlayData& Overlay = OverlayData->Database[I];
		Overlay.Widget = CreateWidget<UDebugOverlay>(PlayerController, OverlayData->OverlayClass);
		if (!Overlay.Widget) continue;

		Overlay.Widget->SetTitle(Overlay.Title);

		for (FDebugCategoryData& Category : Overlay.Categories)
		{
			Category.Widget = CreateWidget<UDebugOverlay>(PlayerController, OverlayData->CategoryClass);
			if (!Category.Widget) continue;

			Category.Widget->SetTitle(Category.Title);

			for (auto Value = Category.DebugValues.begin(); Value != Category.DebugValues.end(); ++Value)
			{
				Value->Value.Widget = CreateWidget<UDebugValue>(PlayerController, OverlayData->ValueClass);
				if (!Value->Value.Widget) continue;

				Value->Value.Widget->SetName(Value->Key);
				Category.Widget->AddToContainer(Value->Value.Widget);
			}

			Overlay.Widget->AddToContainer(Category.Widget);
		}

		Overlay.Widget->AddToViewport();

		if (Overlay.Title == OverlayName)
		{
			bOverlayNameFound = true;
			CurrentOverlay = &Overlay;
			CurrentOverlayIdx = I;
			CurrentOverlay->Widget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Overlay.Widget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	bTick = true;
	if (!bOverlayNameFound)
	{
		CurrentOverlay = &OverlayData->Database[0];
		OverlayData->Database[0].Widget->SetVisibility(ESlateVisibility::Visible);
		return EDebugOverlayInit::ENoneExist;
	}

	return EDebugOverlayInit::ESuccess;
#endif
}

void UDebugOverlaySubsystem::AttachDefaultValues()
{
	AttachToOverlay("General", "Performance and Balance", "FPS", 
		FOnOverlayUpdateInt32::CreateUFunction(this, "GetFPS"));
}

void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateBool GetValue, FOnOverlayResetValue ResetValue)
{
#if UE_WITH_CHEAT_MANAGER
	FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName);
	if (Value && GetValue.IsBound())
	{
		Value->Type = EBool;
		Value->GetBool = MoveTemp(GetValue);

		if (ResetValue.IsBound()) Value->ResetValue = ResetValue;
	}
#endif
}

void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateInt32 GetValue, FOnOverlayResetValue ResetValue)
{
#if UE_WITH_CHEAT_MANAGER
	FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName);
	if (Value && GetValue.IsBound())
	{
		Value->Type = EInt;
		Value->GetInt32 = MoveTemp(GetValue);
		
		if (ResetValue.IsBound()) Value->ResetValue = ResetValue;
	}
#endif
}

void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateFloat GetValue, FOnOverlayResetValue ResetValue)
{
#if UE_WITH_CHEAT_MANAGER
	FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName);
	if (Value && GetValue.IsBound())
	{
		Value->Type = EFloat;
		Value->GetFloat = MoveTemp(GetValue);

		if (ResetValue.IsBound()) Value->ResetValue = ResetValue;
	}
#endif
}

void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateVector2 GetValue, FOnOverlayResetValue ResetValue)
{
#if UE_WITH_CHEAT_MANAGER
	FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName);
	if (Value && GetValue.IsBound())
	{
		Value->Type = EVector2;
		Value->GetVector2 = MoveTemp(GetValue);

		if (ResetValue.IsBound()) Value->ResetValue = ResetValue;
	}
#endif
}

void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateVector3 GetValue, FOnOverlayResetValue ResetValue)
{
#if UE_WITH_CHEAT_MANAGER
	FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName);
	if (Value && GetValue.IsBound())
	{
		Value->Type = EVector3;
		Value->GetVector3 = MoveTemp(GetValue);

		if (ResetValue.IsBound()) Value->ResetValue = ResetValue;
	}
#endif
}

void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateVector4 GetValue, FOnOverlayResetValue ResetValue)
{
#if UE_WITH_CHEAT_MANAGER
	FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName);
	if (Value && GetValue.IsBound())
	{
		Value->Type = EVector4;
		Value->GetVector4 = MoveTemp(GetValue);

		if (ResetValue.IsBound()) Value->ResetValue = ResetValue;
	}
#endif
}

void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateString GetValue, FOnOverlayResetValue ResetValue)
{
#if UE_WITH_CHEAT_MANAGER
	FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName);
	if (Value && GetValue.IsBound())
	{
		Value->Type = EString;
		Value->GetString = MoveTemp(GetValue);

		if (ResetValue.IsBound()) Value->ResetValue = ResetValue;
	}
#endif
}

void UDebugOverlaySubsystem::ChangeOverlay(const int32 NewIdx)
{
	CurrentOverlay->Widget->SetVisibility(ESlateVisibility::Hidden);
	CurrentOverlayIdx = (NewIdx % OverlayData->Database.Num() + OverlayData->Database.Num()) % OverlayData->Database.Num();
	CurrentOverlay = &OverlayData->Database[CurrentOverlayIdx];
	CurrentOverlay->Widget->SetVisibility(ESlateVisibility::Visible);
}

void UDebugOverlaySubsystem::GetOverlayNames(TArray<FString>& OverlayNames)
{
#if UE_WITH_CHEAT_MANAGER
	if (!OverlayData) return;

	OverlayNames.Reset();
	for (const FDebugOverlayData& Overlay : OverlayData->Database)
		OverlayNames.Add(Overlay.Title.ToString());
#endif
}

void UDebugOverlaySubsystem::SetValue(FDebugValueData& Value)
{
	if (!IsValid(Value.Widget)) return;

	switch (Value.Type)
	{
	case EDebugValueType::EBool:
		Value.GetBool.IsBound() ? Value.Widget->SetValue(Value.GetBool.Execute()) : Value.Widget->SetNull();
		break;
	case EDebugValueType::EInt:
		Value.GetInt32.IsBound() ? Value.Widget->SetValue(Value.GetInt32.Execute()) : Value.Widget->SetNull();
		break;
	case EDebugValueType::EFloat:
		Value.GetFloat.IsBound() ? Value.Widget->SetValue(Value.GetFloat.Execute()) : Value.Widget->SetNull();
		break;
	case EDebugValueType::EVector2:
		Value.GetVector2.IsBound() ? Value.Widget->SetValue(Value.GetVector2.Execute()) : Value.Widget->SetNull();
		break;
	case EDebugValueType::EVector3:
		Value.GetVector3.IsBound() ? Value.Widget->SetValue(Value.GetVector3.Execute()) : Value.Widget->SetNull();
		break;
	case EDebugValueType::EVector4:
		Value.GetVector4.IsBound() ? Value.Widget->SetValue(Value.GetVector4.Execute()) : Value.Widget->SetNull();
		break;
	case EDebugValueType::EString:
		Value.GetString.IsBound() ? Value.Widget->SetValue(Value.GetString.Execute()) : Value.Widget->SetNull();
	}

	Value.ResetValue.ExecuteIfBound();
}

FDebugValueData* UDebugOverlaySubsystem::FindKey(const FName& OverlayName, const FName& CategoryName, const FName& VariableName)
{
	FDebugValueData** Value = OverlayFrontend.Find(TTuple<FName, FName, FName>(OverlayName, CategoryName, VariableName));
	if (!Value)
	{
		UE_LOG(LogDebugOverlaySubsystem, Warning, TEXT("AttachToOverlay failed; variable does not exist or incorrect key(s)!"));
		return nullptr;
	}
	return *Value;
}