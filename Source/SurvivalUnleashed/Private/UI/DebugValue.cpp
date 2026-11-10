// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DebugValue.h"

DEFINE_LOG_CATEGORY_STATIC(LogDebugValue, Log, All)

void UDebugValue::SetName(const FName& String)
{
	VarName->SetText(FText::FromName(String));
}

void UDebugValue::SetValue(const TWeakObjectPtr<bool> Value)
{
	if (!Value.IsValid())
	{
		SetNull(Val1);
		return;
	}

	Val1->SetText(FText::FromString(Value.Get() ? "true" : "false"));
}

void UDebugValue::SetValue(const TWeakObjectPtr<int32> Value)
{
	if (!Value.IsValid())
	{
		SetNull(Val1);
		return;
	}

	Val1->SetText(FText::FromString(FString::FromInt(*Value)));
}

void UDebugValue::SetValue(const TWeakObjectPtr<float> Value)
{
	if (!Value.IsValid())
	{
		SetNull(Val1);
		return;
	}

	Val1->SetText(FText::FromString(FString::SanitizeFloat(*Value, 6)));
}

void UDebugValue::SetValue(const TWeakObjectPtr<FVector2D> Value)
{
	if (!Value.IsValid())
	{
		SetNull(Val1);
		SetNull(Val2);
		return;
	}

	Val1->SetText(FText::FromString(FString::SanitizeFloat(Value->X, 6)));
	Val2->SetText(FText::FromString(FString::SanitizeFloat(Value->Y, 6)));
}

void UDebugValue::SetValue(const TWeakObjectPtr<FVector> Value)
{
	if (!Value.IsValid())
	{
		SetNull(Val1);
		SetNull(Val2);
		SetNull(Val3);
		return;
	}

	Val1->SetText(FText::FromString(FString::SanitizeFloat(Value->X, 6)));
	Val2->SetText(FText::FromString(FString::SanitizeFloat(Value->Y, 6)));
	Val3->SetText(FText::FromString(FString::SanitizeFloat(Value->Z, 6)));
}

void UDebugValue::SetValue(const TWeakObjectPtr<FVector4> Value)
{
	if (!Value.IsValid())
	{
		SetNull(Val1);
		SetNull(Val2);
		SetNull(Val3);
		SetNull(Val4);
		return;
	}

	Val1->SetText(FText::FromString(FString::SanitizeFloat(Value->X, 6)));
	Val2->SetText(FText::FromString(FString::SanitizeFloat(Value->Y, 6)));
	Val3->SetText(FText::FromString(FString::SanitizeFloat(Value->Z, 6)));
	Val4->SetText(FText::FromString(FString::SanitizeFloat(Value->W, 6)));
}

void UDebugValue::SetValue(const TWeakObjectPtr<FString> Value)
{
	if (!Value.IsValid())
	{
		SetNull(Val1);
		return;
	}

	Val1->SetText(FText::FromString(*Value));
}