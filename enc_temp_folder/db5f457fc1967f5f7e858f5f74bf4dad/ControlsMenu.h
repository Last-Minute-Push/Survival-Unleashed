// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/InputKeySelector.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Framework/Commands/InputChord.h"
#include "ControlsMenu.generated.h"

/**
 * 
 */

UCLASS()
class SURVIVALUNLEASHED_API UControlsMenu : public UUserWidget
{
	GENERATED_BODY()
private:
	virtual void NativeOnInitialized() override;
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UInputKeySelector* ForwardKeySelect;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UInputKeySelector* RightKeySelect;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UInputKeySelector* LeftKeySelect;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UInputKeySelector* DownKeySelect;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UInputKeySelector* DashKeySelect;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UInputMappingContext* InputMapContext;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* Moves;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* Dashes;
public:
	UFUNCTION()
	void SetForward(FInputChord SelectedInput);
	UFUNCTION()
	void SetLeft(FInputChord SelectedInput);
	UFUNCTION()
	void SetRight(FInputChord SelectedInput);
	UFUNCTION()
	void SetDown(FInputChord SelectedInput);
	UFUNCTION()
	void SetDash(FInputChord SelectedInput);
};
