// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DebugValue.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

DEFINE_LOG_CATEGORY_STATIC(LogDebugValue, Log, All)

void UDebugValue::SetName(const FName& String)
{
	VarName->SetText(FText::FromName(String));
}

void UDebugValue::SetNull()
{
	Val1->SetText(FText::FromString("null"));
	Val2->SetText(FText::FromString(""));
	Val3->SetText(FText::FromString(""));
	Val4->SetText(FText::FromString(""));
}

void UDebugValue::SetValue(const bool& Value)
{
	Val1->SetText(FText::FromString(Value ? "true" : "false"));
}

void UDebugValue::SetValue(const int32& Value)
{
	Val1->SetText(FText::FromString(FString::FromInt(Value)));
}

void UDebugValue::SetValue(const float& Value)
{
	Val1->SetText(FText::FromString(FString::SanitizeFloat(Value, 6)));
}

void UDebugValue::SetValue(const FVector2D& Value)
{
	Val1->SetText(FText::FromString(FString::SanitizeFloat(Value.X, 6)));
	Val2->SetText(FText::FromString(FString::SanitizeFloat(Value.Y, 6)));
}

void UDebugValue::SetValue(const FVector& Value)
{
	Val1->SetText(FText::FromString(FString::SanitizeFloat(Value.X, 6)));
	Val2->SetText(FText::FromString(FString::SanitizeFloat(Value.Y, 6)));
	Val3->SetText(FText::FromString(FString::SanitizeFloat(Value.Z, 6)));
}

void UDebugValue::SetValue(const FVector4& Value)
{
	Val1->SetText(FText::FromString(FString::SanitizeFloat(Value.X, 6)));
	Val2->SetText(FText::FromString(FString::SanitizeFloat(Value.Y, 6)));
	Val3->SetText(FText::FromString(FString::SanitizeFloat(Value.Z, 6)));
	Val4->SetText(FText::FromString(FString::SanitizeFloat(Value.W, 6)));
}

void UDebugValue::SetValue(const FString& Value)
{
	Val1->SetText(FText::FromString(Value));
}

void UDebugValue::SetBackgroundColor(FLinearColor Color)
{
	Background->SetBrushColor(Color);
}