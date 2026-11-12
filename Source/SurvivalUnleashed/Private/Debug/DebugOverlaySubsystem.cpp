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

	for (FDebugCategoryData& Category : CurrentOverlay->Categories)
		for (auto Value = Category.DebugValues.begin(); Value != Category.DebugValues.end(); ++Value)
			SetValue(Value->Value);
#endif
}

EDebugOverlayInit UDebugOverlaySubsystem::InitializeDebugOverlay(const FString& OverlayName, const bool bUpdatePerFrame/*= true*/)
{
#if UE_WITH_CHEAT_MANAGER
	if (!IsValid(OverlayData) || OverlayFrontend.IsEmpty() || !IsValid(PlayerController) ||
		!IsValid(OverlayData->OverlayClass) || !IsValid(OverlayData->CategoryClass) || !IsValid(OverlayData->ValueClass))
	{
		UE_LOG(LogDebugOverlaySubsystem, Error, TEXT("There is no debug overlay database or invalid initialization; debug ovelay will not display!"));
		return EDebugOverlayInit::EFailed;
	}

	bPerFrame = bUpdatePerFrame;
	bool bOverlayNameFound = false;

	for (FDebugOverlayData& Overlay : OverlayData->Database)
	{
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

void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateBool GetBool)
{
#if UE_WITH_CHEAT_MANAGER
	if (FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName))
	{
		Value->Type = EBool;
		Value->GetBool = MoveTemp(GetBool);
	}
#endif
}

void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateInt32 GetInt32)
{
#if UE_WITH_CHEAT_MANAGER
	if (FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName))
	{
		Value->Type = EInt;
		Value->GetInt32 = MoveTemp(GetInt32);
	}
#endif
}

void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateFloat GetFloat)
{
#if UE_WITH_CHEAT_MANAGER
	if (FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName))
	{
		Value->Type = EFloat;
		Value->GetFloat = MoveTemp(GetFloat);
	}
#endif
}

void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateVector2 GetVector2)
{
#if UE_WITH_CHEAT_MANAGER
	if (FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName))
	{
		Value->Type = EVector2;
		Value->GetVector2 = MoveTemp(GetVector2);
	}
#endif
}

void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateVector3 GetVector3)
{
#if UE_WITH_CHEAT_MANAGER
	if (FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName))
	{
		Value->Type = EVector3;
		Value->GetVector3 = MoveTemp(GetVector3);
	}
#endif
}

void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateVector4 GetVector4)
{
#if UE_WITH_CHEAT_MANAGER
	if (FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName))
	{
		Value->Type = EVector4;
		Value->GetVector4 = MoveTemp(GetVector4);
	}
#endif
}

void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateString GetString)
{
#if UE_WITH_CHEAT_MANAGER
	if (FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName))
	{
		Value->Type = EString;
		Value->GetString = MoveTemp(GetString);
	}
#endif
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