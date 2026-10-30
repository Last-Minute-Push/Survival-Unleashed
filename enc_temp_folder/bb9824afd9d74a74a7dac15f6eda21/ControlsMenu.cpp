// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ControlsMenu.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "EnhancedInputComponent.h"

void UControlsMenu::NativeOnInitialized()
{
	if (ForwardKeySelect && RightKeySelect && LeftKeySelect && DownKeySelect && DashKeySelect) {
		ForwardKeySelect->OnKeySelected.AddDynamic(this, &UControlsMenu::SetForward);
		LeftKeySelect->OnKeySelected.AddDynamic(this, &UControlsMenu::SetLeft);
		RightKeySelect->OnKeySelected.AddDynamic(this, &UControlsMenu::SetRight);
		DownKeySelect->OnKeySelected.AddDynamic(this, &UControlsMenu::SetDown);
		DashKeySelect->OnKeySelected.AddDynamic(this, &UControlsMenu::SetDash);
	}
}

void UControlsMenu::SetForward(FInputChord SelectedInput)
{
	TArray<FEnhancedActionKeyMapping> AllMappings = InputMapContext->GetMappings();
	for (const FEnhancedActionKeyMapping& Mapping : AllMappings) {
		if (Mapping.Action == Moves) {
			FEnhancedActionKeyMapping NewMapping;
			NewMapping.Action = Moves;
			NewMapping.Key = SelectedInput.Key;
			InputMapContext->GetMapping(0) = NewMapping;
			break;
		}
	}
}

void UControlsMenu::SetLeft(FInputChord SelectedInput)
{
	TArray<FEnhancedActionKeyMapping> AllMappings = InputMapContext->GetMappings();
	for (const FEnhancedActionKeyMapping& Mapping : AllMappings) {
		if (Mapping.Action == Moves) {
			FEnhancedActionKeyMapping NewMapping;
			NewMapping.Action = Moves;
			NewMapping.Key = SelectedInput.Key;
			InputMapContext->GetMapping(2) = NewMapping;
			break;
		}
	}
}

void UControlsMenu::SetRight(FInputChord SelectedInput)
{
	TArray<FEnhancedActionKeyMapping> AllMappings = InputMapContext->GetMappings();
	for (const FEnhancedActionKeyMapping& Mapping : AllMappings) {
		if (Mapping.Action == Moves) {
			FEnhancedActionKeyMapping NewMapping;
			NewMapping.Action = Moves;
			NewMapping.Key = SelectedInput.Key;
			InputMapContext->GetMapping(3) = NewMapping;
			break;
		}
	}
}

void UControlsMenu::SetDown(FInputChord SelectedInput)
{
	TArray<FEnhancedActionKeyMapping> AllMappings = InputMapContext->GetMappings();
	for (const FEnhancedActionKeyMapping& Mapping : AllMappings) {
		if (Mapping.Action == Moves) {
			FEnhancedActionKeyMapping NewMapping;
			NewMapping.Action = Moves;
			NewMapping.Key = SelectedInput.Key;
			InputMapContext->GetMapping(1) = NewMapping;
			break;
		}
	}
}

void UControlsMenu::SetDash(FInputChord SelectedInput)
{
	TArray<FEnhancedActionKeyMapping> AllMappings = InputMapContext->GetMappings();
	for (const FEnhancedActionKeyMapping& Mapping : AllMappings) {
		if (Mapping.Action == Dashes) {
			FEnhancedActionKeyMapping NewMapping;
			NewMapping.Action = Dashes;
			NewMapping.Key = SelectedInput.Key;
			InputMapContext->GetMapping(0) = NewMapping;
			break;
		}
	}
}
