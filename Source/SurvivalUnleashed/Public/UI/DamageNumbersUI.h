// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/HUDBase.h"
#include "DamageNumbersUI.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UDamageNumbersUI : public UHUDBase
{
	GENERATED_BODY()

public:
	// Shows Damage as a whole number. Position, rise, and fade are handled by UDamageNumberLayer.
	void SetDamage(float Damage);
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* DamageTextBlock;
};
