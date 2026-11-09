#include "UI/UpgradeCard.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Weapons/WeaponData.h"
#include "Audio/Interfaces/AudioManagerInterface.h"

UUpgradeCard::UUpgradeCard()
{
	ZOrderLayer = 2;
}

void UUpgradeCard::NativePreConstruct() {
	Super::NativePreConstruct();
	SetButtonClickSFX(ButtonClickedSFX);
}
void UUpgradeCard::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	PickButton->OnClicked.AddDynamic(this, &UUpgradeCard::HandleClicked); // once per widget, so no double-binding
	PickButton->OnHovered.AddDynamic(this, &UUpgradeCard::HandleHoverSFX);
	
}

void UUpgradeCard::SetOffer(const FUpgradeOffer& InOffer)
{
	Offer = InOffer;

	// Look up what to show for this offer
	const FUpgradeCardInfo* Info = &HealInfo;
	if (Offer.Kind == EUpgradeKind::Stat)
	{
		Info = StatInfo.Find(Offer.Stat);
		WeaponIconSwitcher->SetVisibility(ESlateVisibility::Hidden);
	}
	else if (Offer.Kind == EUpgradeKind::Weapon)
	{
		Info = WeaponInfo.Find(Offer.Weapon);
		GetWeaponImageIndex(Offer.Weapon);
	}

	if (Info)
	{
		NameText->SetText(Info->Name);
	}
	else // missing row in Class Defaults: still show something readable
	{
		NameText->SetText(Offer.Kind == EUpgradeKind::Weapon
			? StaticEnum<EWeaponCategory>()->GetDisplayNameTextByIndex(static_cast<int64>(Offer.Weapon))
			: StaticEnum<EPlayerStats>()->GetDisplayNameTextByValue(static_cast<int64>(Offer.Stat)));
	}

	if (Offer.Kind == EUpgradeKind::Heal)
	{
		LevelText->SetText(FText::GetEmpty());
	}
	else if (Offer.IsNew())
	{
		LevelText->SetText(INVTEXT("NEW"));
	}
	else
	{
		LevelText->SetText(FText::Format(INVTEXT("Lv {0} -> {1}"), Offer.CurrentLevel, Offer.CurrentLevel + 1));
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(Info ? Info->Description : FText::GetEmpty());
	}

}

void UUpgradeCard::HandleClicked()
{
	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{
		if (CurrentButtonClicked) {

			I->PlaySound(CurrentButtonClicked);
		}
		else
		{
			//SFX null
		}
	}
	else
	{
		// Audio Lib Found!
	}
	OnPicked.ExecuteIfBound(CardIndex);
}
void UUpgradeCard::HandleHoverSFX() {
	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{
		if (ButtonHoverSFX) {
			I->PlaySound(ButtonHoverSFX);
		}
		else
		{
			//SFX null
		}
	}
	else
	{
		// Audio Lib Found!
	}
}
void UUpgradeCard::SetButtonClickSFX(USoundBase* SFX)
{
	CurrentButtonClicked = SFX;
}

void UUpgradeCard::GetWeaponImageIndex(EWeaponCategory Category)
{


	switch (Category)
	{
	case EWeaponCategory::EBP_Sword:
		WeaponIconSwitcher->SetActiveWidgetIndex(1);
		//return 1;
		break;
	case EWeaponCategory::EBP_Gauntlets:
		WeaponIconSwitcher->SetActiveWidgetIndex(2);
		//return 2;
		break;
	case EWeaponCategory::EBP_Axe:
		WeaponIconSwitcher->SetActiveWidgetIndex(3);
		//return 3;
		break;
	case EWeaponCategory::EBP_Spear:
		WeaponIconSwitcher->SetActiveWidgetIndex(4);
		//return 4;
		break;
	case EWeaponCategory::EBP_BowToGun:
		WeaponIconSwitcher->SetActiveWidgetIndex(5);
		//return 5;
	case EWeaponCategory::EBP_IceStaff:
		WeaponIconSwitcher->SetActiveWidgetIndex(6);
		//return 6;
		break;
	case EWeaponCategory::EBP_EarthStaff:
		WeaponIconSwitcher->SetActiveWidgetIndex(7);
		//return 7;
		break;
	case EWeaponCategory::EBP_FireStaff:
		WeaponIconSwitcher->SetActiveWidgetIndex(8);
		//return 8;
		break;
	case EWeaponCategory::EBP_LightningStaff:
		WeaponIconSwitcher->SetActiveWidgetIndex(9);
		//return 9;
		break;
	}
	//return 0;
}
