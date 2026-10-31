// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OptionsSliderBase.h"
#include "SAudioSlider.h"

UOptionsSliderBase::UOptionsSliderBase()
{
	OutputRange = FVector2D(0.f, 100.f);
	UnitsText = FText::FromString("%");
	Orientation = EOrientation::Orient_Horizontal;
	Value = 1.f;
}

void UOptionsSliderBase::SetSliderValue(float InValue)
{
	MyAudioSlider->SetSliderValue(InValue);
}