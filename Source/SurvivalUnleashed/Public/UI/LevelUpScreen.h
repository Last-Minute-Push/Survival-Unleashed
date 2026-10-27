// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Utility/UpgradeOffers.h"
#include "LevelUpScreen.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSetOwnedWeapons, TArray<FOwnedWeapon>&, WeaponPool);

/**
 *
 */
UCLASS()
class SURVIVALUNLEASHED_API ULevelUpScreen : public UUserWidget
{
	GENERATED_BODY()
private:
	virtual void NativeOnInitialized() override;

	void HandleCardPicked(int32 CardIndex);

	UPROPERTY()
	TArray<class UUpgradeCard*> Cards; // Card1-4, so the code can loop instead of repeating itself

	bool bOfferTaken = false; // ignores a second click before the screen closes

public:
	UPROPERTY()
	class AUC_Player* CurrentPlayer;
	UPROPERTY()
	class UPlayerStatsComponent* CurrentStatsComp;

	void EnableLevelUpScreen();
	void EnabledFocusScreen();

	void SetFourUpgrades(); // generates the offers and fills the four cards
	bool UpgradeLevelDone = false;
	bool UpgradeFocusDone = false;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnSetOwnedWeapons OnSetOwnedWeapons;

	// Every weapon a level-up can offer. Leave empty until the weapon inventory (514) is hooked up: offers are stats only until then.
	UPROPERTY(EditDefaultsOnly, Category = "Level Up")
	TArray<EWeaponCategory> WeaponPool;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UHorizontalBox* HorizontalBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeCard* Card1;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeCard* Card2;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeCard* Card3;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UUpgradeCard* Card4;

	//Focus Upgrade Trio!
#pragma region FocusUpgradeTrioSection
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UVerticalBox* VerticalFocusOptionBox1;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButtonBase* UpgradeIntelligence;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UVerticalBox* VerticalFocusOptionBox2;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButtonBase* UpgradeBarbaric;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UVerticalBox* VerticalFocusOptionBox3;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButtonBase* UpgradeModern;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IntelligenceImage;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* BarbaricImage;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* ModernImage;
#pragma endregion
};
