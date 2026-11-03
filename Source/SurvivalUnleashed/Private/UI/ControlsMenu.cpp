// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ControlsMenu.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "Actor/UC_Player.h"
#include "EnhancedInputComponent.h"
#include "Framework/UGI_SUGameInstance.h"

void UControlsMenu::NativeOnInitialized()
{
	if (ForwardKeySelect && RightKeySelect && LeftKeySelect && DownKeySelect && DashKeySelect) {
		ForwardKeySelect->OnKeySelected.AddDynamic(this, &UControlsMenu::SetForward);
		LeftKeySelect->OnKeySelected.AddDynamic(this, &UControlsMenu::SetLeft);
		RightKeySelect->OnKeySelected.AddDynamic(this, &UControlsMenu::SetRight);
		DownKeySelect->OnKeySelected.AddDynamic(this, &UControlsMenu::SetDown);
		DashKeySelect->OnKeySelected.AddDynamic(this, &UControlsMenu::SetDash);
	}
	UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance());
	GI->ControlsMenu = this;

	TArray<FEnhancedActionKeyMapping>& AllMappings = const_cast<TArray<FEnhancedActionKeyMapping>&>(InputMapContext->GetMappings());
	for (int32 i = 0; i < AllMappings.Num();i++) {
		if (AllMappings[i].Action == Moves) {

			switch (i) {
			case 0:
				ForwardKeySelect->SetSelectedKey(AllMappings[i].Key);
				break;
			case 1:
				DownKeySelect->SetSelectedKey(AllMappings[i].Key);
				break;
			case 2:
				LeftKeySelect->SetSelectedKey(AllMappings[i].Key);
				break;
			case 3:
				RightKeySelect->SetSelectedKey(AllMappings[i].Key);
				break;
			}
		}
		if (AllMappings[i].Action == Dashes) {
			DashKeySelect->SetSelectedKey(AllMappings[i].Key);
		}
	}
}

void UControlsMenu::SetForward(FInputChord SelectedInput)
{
	
	TArray<FEnhancedActionKeyMapping>& AllMappings = const_cast<TArray<FEnhancedActionKeyMapping>&>(InputMapContext->GetMappings());
	for (int32 i = 0; i < AllMappings.Num();i++) {
		if (AllMappings[i].Action == Moves) {
			if (i == 0) {
				AllMappings[i].Key = SelectedInput.Key;
				break;
			}
		}
	}
	UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance());
	GI->UpdatePlayerKeyBinds();
}

void UControlsMenu::SetLeft(FInputChord SelectedInput)
{
	TArray<FEnhancedActionKeyMapping>& AllMappings = const_cast<TArray<FEnhancedActionKeyMapping>&>(InputMapContext->GetMappings());
	for (int32 i = 0; i < AllMappings.Num();i++) {
		if (AllMappings[i].Action == Moves) {
			if (i == 2) {
				AllMappings[i].Key = SelectedInput.Key;
				break;
			}
		}
	}
	UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance());
	GI->UpdatePlayerKeyBinds();
}

void UControlsMenu::SetRight(FInputChord SelectedInput)
{
	TArray<FEnhancedActionKeyMapping>& AllMappings = const_cast<TArray<FEnhancedActionKeyMapping>&>(InputMapContext->GetMappings());
	for (int32 i = 0; i < AllMappings.Num();i++) {
		if (AllMappings[i].Action == Moves) {
			if (i == 3) {
				AllMappings[i].Key = SelectedInput.Key;
				break;
			}
		}
	}
	UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance());
	GI->UpdatePlayerKeyBinds();
}

void UControlsMenu::SetDown(FInputChord SelectedInput)
{
	TArray<FEnhancedActionKeyMapping>& AllMappings = const_cast<TArray<FEnhancedActionKeyMapping>&>(InputMapContext->GetMappings());
	for (int32 i = 0; i < AllMappings.Num();i++) {
		if (AllMappings[i].Action == Moves) {
			if (i == 1) {
				AllMappings[i].Key = SelectedInput.Key;
				break;
			}
		}
	}
	UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance());
	GI->UpdatePlayerKeyBinds();
}

void UControlsMenu::SetDash(FInputChord SelectedInput)
{
	TArray<FEnhancedActionKeyMapping>& AllMappings = const_cast<TArray<FEnhancedActionKeyMapping>&>(InputMapContext->GetMappings());
	for (int32 i = 0; i < AllMappings.Num();i++) {
		if (AllMappings[i].Action == Dashes) {
			{
				AllMappings[i].Key = SelectedInput.Key;
				break;
			}
		}
	}
	UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance());
	GI->UpdatePlayerKeyBinds();
}

void UControlsMenu::UpdatePlayerMappings()
{
	APlayerController* PC = Cast<APlayerController>(PlayerItself->GetOwner());
	if (PC)
	{
		ULocalPlayer* LP = PC->GetLocalPlayer();
		if (LP)
		{
			if (UEnhancedInputLocalPlayerSubsystem* SS = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				SS->RequestRebuildControlMappings();
			}
		}
	}
	UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(GetGameInstance());
	GI->UpdatePlayerKeyBinds();
}
