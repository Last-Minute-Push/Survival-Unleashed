// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "DebugValue.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UDebugValue : public UUserWidget
{
	GENERATED_BODY()

	inline void SetNull(TObjectPtr<UTextBlock> Text) { Text->SetText(FText::FromString("null")); }
	
protected:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> VarName;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Val1;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Val2;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Val3;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Val4;

public:
	void SetName(const FName& String);

	void SetValue(const TWeakObjectPtr<bool> Value);
	void SetValue(const TWeakObjectPtr<int32> Value);
	void SetValue(const TWeakObjectPtr<float> Value);
	void SetValue(const TWeakObjectPtr<FVector2D> Value);
	void SetValue(const TWeakObjectPtr<FVector> Value);
	void SetValue(const TWeakObjectPtr<FVector4> Value);
	void SetValue(const TWeakObjectPtr<FString> Value);
};
