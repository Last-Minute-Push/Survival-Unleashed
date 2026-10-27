// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageNumbersUI.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UDamageNumbersUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// Shows Damage as a whole number. Position, rise, and fade are handled by UDamageNumberLayer.
	void SetDamage(float Damage);
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* DamageTextBlock;

};
