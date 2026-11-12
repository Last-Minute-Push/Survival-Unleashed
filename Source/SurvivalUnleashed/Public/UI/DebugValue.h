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
	
protected:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> VarName;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Val1;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Val2;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Val3;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Val4;

public:
	void SetName(const FName& String);

	void SetNull();
	void SetValue(const bool& Value);

	void SetValue(const int32& Value);

	void SetValue(const float& Value);

	void SetValue(const FVector2D& Value);

	void SetValue(const FVector& Value);

	void SetValue(const FVector4& Value);

	void SetValue(const FString& Value);
};
