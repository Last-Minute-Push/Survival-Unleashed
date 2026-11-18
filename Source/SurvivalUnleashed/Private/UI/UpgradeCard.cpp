#include "UI/UpgradeCard.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Weapons/WeaponData.h"
#include "Weapons/WeaponSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Canvas.h"
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

	if (UWeaponSubsystem* WSS = GetWorld()->GetSubsystem<UWeaponSubsystem>())
	{
		WeaponSubsystem = WSS;
		OnSetWeaponOffer.BindUFunction(WeaponSubsystem, "GetWeaponInfo");
	}
}

void UUpgradeCard::SetOffer(const FUpgradeOffer& InOffer)
{
	Offer = InOffer;

	// Look up what to show for this offer
	FUpgradeCardInfo WeaponCardInfo, * Info = &HealInfo, * WeaponUpgradeInfo = nullptr;
	if (Offer.Kind == EUpgradeKind::Stat)
	{
		Info = StatsInfo.Find(Offer.Stat);
		WeaponIconSwitcher->SetVisibility(ESlateVisibility::Hidden);
	}
	else if (Offer.Kind == EUpgradeKind::Weapon)
	{
		// Icons on a per weapon basis could be set via UWeaponSubsystem::GetWeaponInfo (bound to OnSetWeaponOffer).
		// Keep a fallback function in case there is no icon present for a particular weapon upgrade.
		//GetWeaponImageIndex(Offer.Weapon);

		if (OnSetWeaponOffer.IsBound())
		{
			WeaponUpgradeInfo = &(WeaponCardInfo = OnSetWeaponOffer.Execute(Offer.Weapon, Offer.CurrentLevel));
		}

		Info = WeaponsInfo.Find(Offer.Weapon);
	}

	// Set the offer's info

	// Icons
	if (WeaponUpgradeInfo && IsValid(WeaponUpgradeInfo->Icon)) // Weapon Upgrade
	{
		//LastIcon->SetBrushFromTexture(WeaponUpgradeInfo->Icon);
		//WeaponIconSwitcher->SetActiveWidgetIndex(WeaponIconSwitcher->GetNumWidgets() - 1);
		
	}

	// Description
	if (WeaponUpgradeInfo && !WeaponUpgradeInfo->Description.IsEmpty())   // Weapon Upgrade Override
	{
		DescriptionText->SetText(WeaponUpgradeInfo->Description);
	}
	else if (Info && !Info->Description.IsEmpty())                        // Defaults
	{
		DescriptionText->SetText(Info->Description);
	}

	// Name
	if (WeaponUpgradeInfo && !WeaponUpgradeInfo->Name.IsEmpty())          // Weapon Upgrade Override
	{
		NameText->SetText(WeaponUpgradeInfo->Name);
	}
	else if (Info && !Info->Name.IsEmpty())                               // Defaults
	{
		NameText->SetText(Info->Name);
	}
	else // missing row in Class Defaults: still show something readable  // Fallback
	{
		NameText->SetText(Offer.Kind == EUpgradeKind::Weapon
			? StaticEnum<EWeaponCategory>()->GetDisplayNameTextByIndex(static_cast<int64>(Offer.Weapon))
			: StaticEnum<EPlayerStats>()->GetDisplayNameTextByValue(static_cast<int64>(Offer.Stat)));
	}

	// Level
	if (Offer.Kind == EUpgradeKind::Heal)                                 // Heal Override
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
