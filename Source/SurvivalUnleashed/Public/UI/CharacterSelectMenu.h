// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/HUDBase.h"
#include "UI/CharacterButton.h"
#include "Framework/UGI_SUGameInstance.h"
#include "Engine/Texture2D.h"
#include "UI/ButtonBase.h"
#include "CharacterSelectMenu.generated.h"


/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterSelectClosed);
UCLASS()
class SURVIVALUNLEASHED_API UCharacterSelectMenu : public UHUDBase
{
	GENERATED_BODY()
	


	virtual void NativePreConstruct() override;

	virtual void NativeOnInitialized() override;

	void CharacterLock();

	UFUNCTION() void ChooseCharacterV();
	UFUNCTION() void ChooseCharacterF();
	UFUNCTION() void ChooseCharacterR();
	UFUNCTION() void ChooseCharacterW();
	UFUNCTION() void BackButton();

	void CharacterUnlock(UCharacterButton* Character);

	UUGI_SUGameInstance* GI;

public:

	UPROPERTY(BlueprintAssignable)
	FOnCharacterSelectClosed OnBack;


	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) UCharacterButton* CharacterOption_1;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) UCharacterButton* CharacterOption_2;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) UCharacterButton* CharacterOption_3;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) UCharacterButton* CharacterOption_4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)  UTexture2D* CharacterImage2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)  UTexture2D* CharacterImage3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)  UTexture2D* CharacterImage4;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UTextBlock* Points;


	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UButtonBase* BackUpgradeButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) class UButtonBase* PlayButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText PointsInfo;

	float CurrentPointsAmount = 0;

	UFUNCTION() void SetPointsAmount(float CurrentPoints);

	UFUNCTION() void SetPointsText(int Ppoints);

	UPROPERTY(BlueprintAssignable, Category = "Character Select")
	FOnCharacterSelectClosed OnClosed;

};
