// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DebugValue.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UDebugValue : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> VarName;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Val1;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Val2;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Val3;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> Val4;
};
