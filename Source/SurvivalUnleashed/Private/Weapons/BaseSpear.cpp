// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/BaseSpear.h"
#include "Weapons/WeaponData.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpear, Log, All);

void ABaseSpear::BeginPlay()
{
	Super::Super::BeginPlay();

	// Bow Properties
	if (Data)
	{
		Range = Data->Range;
		Damage = Data->Damage;
		Height = Data->Height;
		Radius = Data->DistanceFromPlayer;
		Cooldown = Data->Cooldown * 0.5f;
		Knockback = Data->Knockback;
	}
	else
	{
		UE_LOG(LogSpear, Error, TEXT("ABaseBowToGun's WeaponData asset was not set!"));

		Range = 0.f;
		Damage = 0.f;
		Cooldown = 0.f;
		Knockback = 0.f;
	}

	// Location
	Mesh->AddLocalOffset(FVector(0.f, Radius, Height));
	Mesh->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
}