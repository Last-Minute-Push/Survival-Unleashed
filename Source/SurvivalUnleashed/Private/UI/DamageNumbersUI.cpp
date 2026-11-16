// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/DamageNumbersUI.h"
#include "Components/TextBlock.h"

DEFINE_LOG_CATEGORY_STATIC(LogDamageNumbers, Log, All);

void UDamageNumbersUI::SetDamage(float Damage)
{
	if (DamageTextBlock)
	{
		DamageTextBlock->SetText(FText::AsNumber(FMath::Max(1, FMath::RoundToInt(Damage))));
	}
}