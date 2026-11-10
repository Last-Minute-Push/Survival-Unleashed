// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DebugOverlayDatabase.generated.h"

class UDebugOverlay;
class UDebugCategory;
class UDebugValue;

UENUM(BlueprintType)
enum EDebugValueType : uint8
{
	EBool   UMETA(DisplayName = "Bool"),
	EInt    UMETA(DisplayName = "Int"),
	EFloat  UMETA(DisplayName = "Float"),
	EFloat2 UMETA(DisplayName = "Float2"),
	EFloat3 UMETA(DisplayName = "Float3"),
	EString UMETA(DisplayName = "String")
};

USTRUCT(BlueprintType)
struct FDebugValueData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere) TObjectPtr<UDebugValue> Widget;
	UPROPERTY(EditAnywhere) TEnumAsByte<EDebugValueType> Type;

	union
	{
		TWeakObjectPtr<bool> BoolValue;
		TWeakObjectPtr<int> IntValue;
		TWeakObjectPtr<float> FloatValue;
		TWeakObjectPtr<FString> StringValue;

		TWeakObjectPtr<float> X;
	};
	TWeakObjectPtr<float> Y;
	TWeakObjectPtr<float> Z;
	TWeakObjectPtr<float> W;

	FDebugValueData() {};
};

USTRUCT(BlueprintType)
struct FDebugCategoryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)	FName Name;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UDebugCategory> Wiget;
	UPROPERTY(EditAnywhere) TMap<FName, FDebugValueData> DebugValues;
};

USTRUCT(BlueprintType)
struct FDebugOverlayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName Title;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UDebugOverlay> Widget;
	UPROPERTY(EditAnywhere) TArray<FDebugCategoryData> Categories;
};

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UDebugOverlayDatabase : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly) TArray<FDebugOverlayData> Database;
};
