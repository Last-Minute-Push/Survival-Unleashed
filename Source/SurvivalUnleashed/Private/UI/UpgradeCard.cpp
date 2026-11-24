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
	Icon->SetVisibility(ESlateVisibility::Hidden);

	// Look up what to show for this offer
	FUpgradeCardInfo WeaponCardInfo, *Info = &HealInfo, *WeaponUpgradeInfo = nullptr;
	if (Offer.Kind == EUpgradeKind::Stat)
	{
		Info = StatsInfo.Find(Offer.Stat);
	}
	else if (Offer.Kind == EUpgradeKind::Weapon)
	{
		if (OnSetWeaponOffer.IsBound())
		{
			WeaponUpgradeInfo = &(WeaponCardInfo = OnSetWeaponOffer.Execute(Offer.Weapon, Offer.CurrentLevel));
		}

		Info = WeaponsInfo.Find(Offer.Weapon);
	}

	// Set the offer's info

	// Icon
	if (WeaponUpgradeInfo && IsValid(WeaponUpgradeInfo->Icon)) // Weapon Upgrade Override
	{
		Icon->SetBrushFromTexture(WeaponUpgradeInfo->Icon);
		Icon->SetVisibility(ESlateVisibility::Visible);
	}
	else if (Info && IsValid(Info->Icon)) // Defaults
	{
		Icon->SetBrushFromTexture(Info->Icon);
		Icon->SetVisibility(ESlateVisibility::Visible);
	}

	// Name
	if (WeaponUpgradeInfo && !WeaponUpgradeInfo->Name.IsEmpty()) // Weapon Upgrade Override
	{
		NameText->SetText(WeaponUpgradeInfo->Name);
	}
	else if (Info && !Info->Name.IsEmpty()) // Defaults
	{
		NameText->SetText(Info->Name);
	}
	else // missing row in Class Defaults: still show something readable // Fallback
	{
		NameText->SetText(Offer.Kind == EUpgradeKind::Weapon
			? StaticEnum<EWeaponCategory>()->GetDisplayNameTextByIndex(static_cast<int64>(Offer.Weapon))
			: StaticEnum<EPlayerStats>()->GetDisplayNameTextByValue(static_cast<int64>(Offer.Stat)));
	}

	// Level
	if (Offer.Kind == EUpgradeKind::Heal) // Heal Override
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

	// Description
	if (WeaponUpgradeInfo && !WeaponUpgradeInfo->Description.IsEmpty()) // Weapon Upgrade Override
	{
		DescriptionText->SetText(WeaponUpgradeInfo->Description);
	}
	else if (Info && !Info->Description.IsEmpty()) // Defaults
	{
		DescriptionText->SetText(Info->Description);
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
