// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUDBase.h"

void UHUDBase::AddToViewport()
{
	Super::AddToViewport(ZOrderLayer);
}

void UHUDBase::AddToViewport(int32 ZOrder)
{
	Super::AddToViewport(ZOrder);
}