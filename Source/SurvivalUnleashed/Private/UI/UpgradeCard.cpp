#include "UI/UpgradeCard.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
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
	}
	else if (Offer.Kind == EUpgradeKind::Weapon)
	{
		Info = WeaponInfo.Find(Offer.Weapon);
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

	if (IconImage)
	{
		UTexture2D* Icon = Info ? Info->Icon.Get() : nullptr;
		IconImage->SetBrushFromTexture(Icon);
		IconImage->SetVisibility(Icon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
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