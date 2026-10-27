// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LevelUpScreen.h"
#include "UI/ButtonBase.h"
#include "UI/UpgradeCard.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Actor/UC_Player.h"
#include "Utility/PlayerStatsComponent.h"
#include "Utility/UpgradeOffers.h"
#include "Components/Image.h"


void ULevelUpScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Cards = { Card1, Card2, Card3, Card4 };
	for (int32 i = 0; i < Cards.Num(); ++i)
	{
		Cards[i]->CardIndex = i;
		Cards[i]->OnPicked.BindUObject(this, &ULevelUpScreen::HandleCardPicked);
	}
}

void ULevelUpScreen::SetFourUpgrades()
{
	if (!CurrentStatsComp) {
		return;

	}

	FBuildSnapshot Build;
	Build.StatLevels = CurrentStatsComp->GetStatRanks();
	Build.WeaponPool = WeaponPool;
	OnSetOwnedWeapons.Broadcast(Build.OwnedWeapons);

	const TArray<FUpgradeOffer> Offers = UpgradeOffers::Generate(Build, Cards.Num());
	for (int32 i = 0; i < Cards.Num(); ++i) {
		Cards[i]->SetOffer(Offers[i]);

	}
	bOfferTaken = false;
}

void ULevelUpScreen::HandleCardPicked(int32 CardIndex)
{
	if (bOfferTaken || !CurrentStatsComp || !Cards.IsValidIndex(CardIndex)) {
		return;

	}
	bOfferTaken = true; // set first: finishing the level-up may open the next one, which clears it again

	const FUpgradeOffer& Offer = Cards[CardIndex]->GetOffer();
	switch (Offer.Kind)
	{
	case EUpgradeKind::Stat:
		CurrentStatsComp->ApplyStatUpgrade(Offer.Stat); // also finishes the level-up
		break;
	case EUpgradeKind::Weapon:
		CurrentStatsComp->OnWeaponUpgrade.Broadcast(Offer.Weapon);
		CurrentStatsComp->FinishUpgrade();
		break;
	case EUpgradeKind::Heal:
		// TODO (561): heal the player
		CurrentStatsComp->FinishUpgrade();
		break;
	}
}

void ULevelUpScreen::EnableLevelUpScreen()
{
	HorizontalBox->SetVisibility(ESlateVisibility::Visible);
	for (UUpgradeCard* Card : Cards) {
		Card->SetVisibility(ESlateVisibility::Visible);

	}

	VerticalFocusOptionBox1->SetVisibility(ESlateVisibility::Hidden);
	IntelligenceImage->SetVisibility(ESlateVisibility::Hidden);
	VerticalFocusOptionBox2->SetVisibility(ESlateVisibility::Hidden);
	BarbaricImage->SetVisibility(ESlateVisibility::Hidden);
	VerticalFocusOptionBox3->SetVisibility(ESlateVisibility::Hidden);
	ModernImage->SetVisibility(ESlateVisibility::Hidden);
}

void ULevelUpScreen::EnabledFocusScreen()
{
	if (CurrentStatsComp) 
	{
		UpgradeIntelligence->CodeButtonClick.AddDynamic(CurrentStatsComp, &UPlayerStatsComponent::LevelUpIntelligence);
		UpgradeBarbaric->CodeButtonClick.AddDynamic(CurrentStatsComp, &UPlayerStatsComponent::LevelUpBarbaric);
		UpgradeModern->CodeButtonClick.AddDynamic(CurrentStatsComp, &UPlayerStatsComponent::LevelUpModern);
	}
	HorizontalBox->SetVisibility(ESlateVisibility::Hidden);
	VerticalFocusOptionBox1->SetVisibility(ESlateVisibility::Visible);
	IntelligenceImage->SetVisibility(ESlateVisibility::Visible);
	VerticalFocusOptionBox2->SetVisibility(ESlateVisibility::Visible);
	BarbaricImage->SetVisibility(ESlateVisibility::Visible);
	VerticalFocusOptionBox3->SetVisibility(ESlateVisibility::Visible);
	ModernImage->SetVisibility(ESlateVisibility::Visible);

	for (UUpgradeCard* Card : Cards) {
		Card->SetVisibility(ESlateVisibility::Hidden);
	}
}