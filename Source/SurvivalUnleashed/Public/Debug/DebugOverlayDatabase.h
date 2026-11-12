// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DebugOverlayDatabase.generated.h"

class UDebugOverlay;
class UDebugCategory;
class UDebugValue;

DECLARE_DELEGATE_RetVal(bool, FOnOverlayUpdateBool)
DECLARE_DELEGATE_RetVal(int32, FOnOverlayUpdateInt32)
DECLARE_DELEGATE_RetVal(float, FOnOverlayUpdateFloat)
DECLARE_DELEGATE_RetVal(FVector2D, FOnOverlayUpdateVector2)
DECLARE_DELEGATE_RetVal(FVector, FOnOverlayUpdateVector3)
DECLARE_DELEGATE_RetVal(FVector4, FOnOverlayUpdateVector4)
DECLARE_DELEGATE_RetVal(FString, FOnOverlayUpdateString)
DECLARE_DELEGATE(FOnOverlayResetValue);

UENUM(BlueprintType)
enum EDebugValueType : uint8
{
	EBool   UMETA(DisplayName = "Bool"),
	EInt    UMETA(DisplayName = "Int"),
	EFloat  UMETA(DisplayName = "Float"),
	EVector2 UMETA(DisplayName = "Vector2"),
	EVector3 UMETA(DisplayName = "Vector3"),
	EVector4 UMETA(DisplayName = "Vector4"),
	EString UMETA(DisplayName = "String")
};

USTRUCT(BlueprintType)
struct FDebugValueData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere) TObjectPtr<UDebugValue> Widget;
	UPROPERTY(EditAnywhere)    TEnumAsByte<EDebugValueType> Type;
	UPROPERTY(EditAnywhere)    bool bUpdatePerSecond;

	FOnOverlayUpdateBool GetBool;
	FOnOverlayUpdateInt32 GetInt32;
	FOnOverlayUpdateFloat GetFloat;
	FOnOverlayUpdateVector2 GetVector2;
	FOnOverlayUpdateVector3 GetVector3;
	FOnOverlayUpdateVector4 GetVector4;
	FOnOverlayUpdateString GetString;
	FOnOverlayResetValue ResetValue;

	FDebugValueData() {};
};

USTRUCT(BlueprintType)
struct FDebugCategoryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)	   FName Title;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UDebugOverlay> Widget;
	UPROPERTY(EditAnywhere)    TMap<FName, FDebugValueData> DebugValues;
};

USTRUCT(BlueprintType)
struct FDebugOverlayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)    FName Title;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UDebugOverlay> Widget;
	UPROPERTY(EditAnywhere)    TArray<FDebugCategoryData> Categories;
};

/**
 * 
 */
UCLASS()
class SURVIVALUNLEASHED_API UDebugOverlayDatabase : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly) TSubclassOf<UUserWidget> OverlayClass;  //! SET IN EDITOR!
	UPROPERTY(EditDefaultsOnly) TSubclassOf<UUserWidget> CategoryClass; //! SET IN EDITOR!
	UPROPERTY(EditDefaultsOnly) TSubclassOf<UUserWidget> ValueClass;    //! SET IN EDITOR!
	UPROPERTY(EditDefaultsOnly) TArray<FDebugOverlayData> Database;     //! SET IN EDITOR!
};
