// Fill out your copyright notice in the Description page of Project Settings.


#include "Debug/DebugOverlaySubsystem.h"
#include "Framework/UGI_SUGameInstance.h"
#include "UI/DebugOverlay.h"
#include "UI/DebugValue.h"
#include "Actor/UC_Player.h"
#include "Enemies/SpawnDirector.h"
#include "GameFramework/CheatManagerDefines.h"
#include "Weapons/BaseWeapon.h"
#include "EngineUtils.h"

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
	Database = DB;

	// We pre-fill the frontend for value linkage in AttachToOverlay()
	for (FDebugOverlayData& Overlay : Database->Overlays)
		for (FDebugCategoryData& Category : Overlay.Categories)
			for (FDebugValueData& Value : Category.Values)
				OverlayFrontend.Add({Overlay.Title, Category.Title, Value.Name}, &Value);
#endif
}

void UDebugOverlaySubsystem::Tick(float DeltaTime)
{
#if UE_WITH_CHEAT_MANAGER
	// FPS is the reciprocal of DeltaTime
	FPS = FMath::TruncToInt32<float>(1.f / DeltaTime);

	PerSecondTimer += DeltaTime;
	bool SecondElapsed = PerSecondTimer >= 1.f;
	if (SecondElapsed) PerSecondTimer = 0.f;
	
	if (!CurrentOverlay) return;

    /** 
     * Updates the overlay with the current values;
	 * should only update the values from the current overlay on display.
	 */
	for (FDebugCategoryData& Category : CurrentOverlay->Categories)
		for (FDebugValueData& Value : Category.Values)
		{
			if (!Value.bUpdatePerSecond)
			{
				SetValue(Value);
			}
			else if (SecondElapsed)
			{
				SetValue(Value);
			}
		}
#endif
}

void UDebugOverlaySubsystem::Deinitialize()
{
	Super::Deinitialize();

	TakeOutTheTrash();
}

EDebugOverlayStatus UDebugOverlaySubsystem::InitializeOverlay(const FString& InOverlay)
{
#if UE_WITH_CHEAT_MANAGER
	if (!IsValid(Database) || OverlayFrontend.IsEmpty() || !IsValid(PlayerController) ||
		!IsValid(Database->OverlayClass) || !IsValid(Database->CategoryClass) || !IsValid(Database->ValueClass))
	{
		UE_LOG(LogDebugOverlaySubsystem, Error, TEXT("There is no debug overlay database or invalid initialization; debug ovelay will not display!"));
		return EDebugOverlayStatus::EFailed;
	}

	bool bMatchFound = false;

	if (!bInitialized) AttachGeneralValues();

    /**
	 * Overlay initialization per world; runs only once when reqested.
	 * Can run again after initialization.
	 * 
	 * Initialization:
	 * Database -> its categories -> its values ->
	 * values contained to their category -> categories contained to their overlay ->
	 * overlay added to the viewport
	 */
	for (int32 I = 0; I < Database->Overlays.Num(); ++I)
	{
		FDebugOverlayData& Overlay = Database->Overlays[I];
		if (!Overlay.Widget.IsValid()) // Only create a widget if there isn't one
		{
			Overlay.Widget = CreateWidget<UDebugOverlay>(PlayerController, Database->OverlayClass);
			if (!Overlay.Widget.IsValid()) continue;

			Overlay.Widget->SetTitle(Overlay.Title);
			Overlay.Widget->SetBackgroundColor(Overlay.BackgroundColor);
		}

		if (Overlay.Title == InOverlay)
		{
			bMatchFound = true;
			LastUsed = CurrentOverlay = &Overlay;
			CurrentOverlayIdx = I;
		}
		else
		{
			Overlay.Widget->SetVisibility(ESlateVisibility::Hidden);
		}

		if (bInitialized) continue; // Skips the rest if it's already initialized

		for (FDebugCategoryData& Category : Overlay.Categories)
		{
			Category.Widget = CreateWidget<UDebugOverlay>(PlayerController, Database->CategoryClass);
			if (!Category.Widget.IsValid()) continue;

			Category.Widget->SetTitle(Category.Title);
			Category.Widget->SetBackgroundColor(Category.BackgroundColor);

			for (FDebugValueData& Value : Category.Values)
			{
				Value.Widget = CreateWidget<UDebugValue>(PlayerController, Database->ValueClass);
				if (!Value.Widget.IsValid()) continue;

				Value.Widget->SetName(Value.Name);
				Value.Widget->SetBackgroundColor(Value.BackgroundColor);

				Category.Widget->AddToContainer(Value.Widget);
			}
			Overlay.Widget->AddToContainer(Category.Widget);
		}
		Overlay.Widget->AddToViewport();
	}
	bInitialized = bTick = true;

	if (!bMatchFound) // Dispalys last used overlay or the first overlay in the array if no match was found
	{
		LastUsed = CurrentOverlay = (LastUsed) ? LastUsed : &Database->Overlays[0];
		CurrentOverlay->Widget->SetVisibility(ESlateVisibility::Visible);
		return  InOverlay != "" ? EDebugOverlayStatus::ENoneExist : EDebugOverlayStatus::ESuccess;
	}

	return EDebugOverlayStatus::ESuccess;
#endif
}


void UDebugOverlaySubsystem::AttachGeneralValues()
{
	// Any other value attachents or bindings that don't belong anywhere else can be placed here.
	for (TActorIterator<ASpawnDirector> It(GetWorld()); It; ++It)
	{
		It->OnAnyEnemyHit.AddDynamic(this, &UDebugOverlaySubsystem::HandleDamagePerSecond);
		break;
	}

	AttachToOverlay("General", "Performance and Balance", "FPS", 
		FOnOverlayUpdateInt32::CreateUFunction(this, "GetFPS"));
}

// Repeated function body with different data types; this shortens the length of this file
#define ATTACH_TO_OVERLAY_BODY(FOnOverlayUpdateX, GetData, DataType) \
void UDebugOverlaySubsystem::AttachToOverlay(const FName& OverlayName, const FName& CategoryName, const FName& VariableName, FOnOverlayUpdateX GetValue, FOnOverlayResetValue ResetValue) \
{ \
	FDebugValueData* Value = FindKey(OverlayName, CategoryName, VariableName); \
	if (Value && !Value->GetData.IsBound() && GetValue.IsBound()) \
	{ \
		Value->Type = DataType; \
		Value->GetData = MoveTemp(GetValue); \
\
		if (ResetValue.IsBound()) Value->ResetValue = ResetValue; \
	} \
}
ATTACH_TO_OVERLAY_BODY(FOnOverlayUpdateBool, GetBool, EBool)
ATTACH_TO_OVERLAY_BODY(FOnOverlayUpdateInt32, GetInt32, EInt)
ATTACH_TO_OVERLAY_BODY(FOnOverlayUpdateFloat, GetFloat, EFloat)
ATTACH_TO_OVERLAY_BODY(FOnOverlayUpdateVector2, GetVector2, EVector2)
ATTACH_TO_OVERLAY_BODY(FOnOverlayUpdateVector3, GetVector3, EVector3)
ATTACH_TO_OVERLAY_BODY(FOnOverlayUpdateVector4, GetVector4, EVector4)
ATTACH_TO_OVERLAY_BODY(FOnOverlayUpdateString, GetString, EString)

void UDebugOverlaySubsystem::GetOverlayNames(TArray<FString>& OverlayNames)
{
#if UE_WITH_CHEAT_MANAGER
	if (!Database) return;

	OverlayNames.Reset();
	for (const FDebugOverlayData& Overlay : Database->Overlays)
		OverlayNames.Add(Overlay.Title.ToString());
#endif
}

void UDebugOverlaySubsystem::TriggerOverlay()
{

	if (bTick) DisableOverlay();
	else InitializeOverlay();
}

void UDebugOverlaySubsystem::DisableOverlay()
{
	if (CurrentOverlay && CurrentOverlay->Widget.IsValid())
	{
		CurrentOverlay->Widget->SetVisibility(ESlateVisibility::Hidden);
	}

	bTick = false;
	CurrentOverlay = nullptr;
}

void UDebugOverlaySubsystem::TakeOutTheTrash()
{
	DisableOverlay();

	LastUsed = nullptr;
	bInitialized = false;

	if (!Database) return;

	for (FDebugOverlayData& Overlay : Database->Overlays)
	{
		for (FDebugCategoryData& Category : Overlay.Categories)
		{
			for (FDebugValueData& Value : Category.Values)
			{
				if (Value.Widget.IsValid())
				{
					Value.Widget->RemoveFromParent();
					Value.Widget->MarkAsGarbage();
					Value.Widget = nullptr;
				}
			}
			if (Category.Widget.IsValid())
			{
				Category.Widget->RemoveFromParent();
				Category.Widget->MarkAsGarbage();
				Category.Widget = nullptr;
			}
		}
		if (Overlay.Widget.IsValid())
		{
			Overlay.Widget->RemoveFromParent();
			Overlay.Widget->MarkAsGarbage();
			Overlay.Widget = nullptr;
		}
	}
}

void UDebugOverlaySubsystem::SwitchOverlay(int32 NewIdx)
{
#if UE_WITH_CHEAT_MANAGER
	if (!IsValid(Database) || !CurrentOverlay || !CurrentOverlay->Widget.IsValid())
	{
		return;
	}

	/**
	 * C languages prefer rounding toward zero via truncated division for early CPU efficiency.
	 * This is fine for cycling forwards only, but cycling in reverse is dangerous because in
	 * -1 % 8, euclidean division would return 7 when truncated division would return -1 instead.
	 *
	 * To compensate, we'd have to handle negative numbers by adding the result to the size of
	 * the array. For any negative number, this will always give us a number within the size
	 * of the array, but for positive numbers, it might overflow the max size.
	 * One extra modulo operation will fix this issue.
	 */
	NewIdx = (NewIdx % Database->Overlays.Num() + Database->Overlays.Num()) % Database->Overlays.Num();
	if (NewIdx == CurrentOverlayIdx) return;

	CurrentOverlayIdx = NewIdx;
	CurrentOverlay->Widget->SetVisibility(ESlateVisibility::Hidden);
	LastUsed = CurrentOverlay = &Database->Overlays[CurrentOverlayIdx];
	CurrentOverlay->Widget->SetVisibility(ESlateVisibility::Visible);
#endif
}

void UDebugOverlaySubsystem::SetOverlayZOrder(int32 ZOrder)
{
	if (!CurrentOverlay || !CurrentOverlay->Widget.IsValid())
	{
		return;
	}

	CurrentOverlay->Widget->SetZOrder(ZOrder);
}

#define SET_VALUE(GetData) Value.GetData.IsBound() ? Value.Widget->SetValue(Value.GetData.Execute()) : Value.Widget->SetNull();
void UDebugOverlaySubsystem::SetValue(FDebugValueData& Value)
{
	if (!Value.Widget.IsValid()) return;

	// Updates the widget to display the current value; displays null if the variable is not available
	switch (Value.Type)
	{
	case EDebugValueType::EBool:
		SET_VALUE(GetBool)
		break;
	case EDebugValueType::EInt:
		SET_VALUE(GetInt32)
		break;
	case EDebugValueType::EFloat:
		SET_VALUE(GetFloat)
		break;
	case EDebugValueType::EVector2:
		SET_VALUE(GetVector2)
		break;
	case EDebugValueType::EVector3:
		SET_VALUE(GetVector3)
		break;
	case EDebugValueType::EVector4:
		SET_VALUE(GetVector4)
		break;
	case EDebugValueType::EString:
		SET_VALUE(GetString)
	}

	Value.ResetValue.ExecuteIfBound();
}

FDebugValueData* UDebugOverlaySubsystem::FindKey(const FName& OverlayName, const FName& CategoryName, const FName& VariableName)
{
#if UE_WITH_CHEAT_MANAGER
	FDebugValueData** Value = OverlayFrontend.Find(TTuple<FName, FName, FName>(OverlayName, CategoryName, VariableName));
	if (!Value)
	{
		UE_LOG(LogDebugOverlaySubsystem, Warning, TEXT("AttachToOverlay failed; variable does not exist or incorrect key(s)!"));
		return nullptr;
	}
	return *Value;
#else
	return nullptr;
#endif
}

void UDebugOverlaySubsystem::HandleDamagePerSecond(AEnemyBase* Enemy, float Damage, AActor* DamageCauser)
{
#if UE_WITH_CHEAT_MANAGER
	if (ABaseWeapon* W = Cast<ABaseWeapon>(DamageCauser))
	{
		/**
		 * We only want to return the actual damage we've dealt to an enemy.
		 * If the HP is less than the damage value (ex. 15 HP and 20 Dmg),
		 * we only add the remaining HP value, not the damage.
		 * 
		 * Since GetCurrentHealth is not clamped and the damage was already applied
		 * to the current health, we add the amount of damage to the current health 
		 * to get the correct HP value before the damage was applied.
		 * 
		 * ex. 15 HP - 20 Dmg = -5 HP <- Not what we want
		 *     -5 HP + 20 Dmg = 15 HP <- Return this instead!
		 */
		W->AddToDamagePerSecond(FMath::Min(Damage, Enemy->GetCurrentHealth() + Damage));
	}
#endif
}

