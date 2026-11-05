#pragma once

#include "CoreMinimal.h"
#include "UI/HUDBase.h"
#include "Weapons/WeaponDatabase.h"
#include "Utility/UpgradeOffers.h"
#include "UpgradeCard.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UTexture2D;

DECLARE_DELEGATE_OneParam(FOnUpgradeCardPicked, int32 /*CardIndex*/);

// What a card shows for one stat or weapon. Filled in on WBP_UpgradeCard's Class Defaults.
USTRUCT(BlueprintType)
struct FUpgradeCardInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FText Name;
	UPROPERTY(EditAnywhere, meta = (MultiLine = true)) FText Description;
	UPROPERTY(EditAnywhere) TObjectPtr<UTexture2D> Icon;
};

// One level-up choice. The level-up screen has four of these and fills each from an FUpgradeOffer.
UCLASS(Abstract)
class SURVIVALUNLEASHED_API UUpgradeCard : public UHUDBase
{
	GENERATED_BODY()

public:
	UUpgradeCard();

	void SetOffer(const FUpgradeOffer& InOffer);
	const FUpgradeOffer& GetOffer() const { return Offer; }

	int32 CardIndex = INDEX_NONE; // set by the level-up screen
	FOnUpgradeCardPicked OnPicked; // native delegate: the screen binds with BindUObject, no UFUNCTION needed on its side

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct() override;

	UPROPERTY(EditDefaultsOnly, Category = "Upgrade Card")
	TMap<EPlayerStats, FUpgradeCardInfo> StatInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Upgrade Card")
	TMap<EWeaponCategory, FUpgradeCardInfo> WeaponInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Upgrade Card")
	FUpgradeCardInfo HealInfo;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UButton> PickButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> NameText;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget)) TObjectPtr<UTextBlock> LevelText;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DescriptionText;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional)) TObjectPtr<UImage> IconImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) USoundBase* ButtonHoverSFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) USoundBase* ButtonClickedSFX;
	USoundBase* CurrentButtonClicked;
private:
	UFUNCTION(BlueprintCallable) // required: UButton::OnClicked is a dynamic delegate
	void HandleClicked();

	UFUNCTION() void HandleHoverSFX();
	UFUNCTION() void SetButtonClickSFX(USoundBase* SFX);

	UPROPERTY()
	FUpgradeOffer Offer;
};
