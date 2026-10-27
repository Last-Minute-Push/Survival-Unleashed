// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponSubsystem.h"
#include "Framework/UGI_SUGameInstance.h"
#include "Weapons/BaseWeapon.h"
#include "Actor/UC_Player.h"

DEFINE_LOG_CATEGORY_STATIC(LogWeaponSubsystem, Log, All);

void UWeaponSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	if (UUGI_SUGameInstance* GI = Cast<UUGI_SUGameInstance>(InWorld.GetGameInstance()))
	{
		if (UWeaponDatabase* DB = GI->GetWeaponDatabase())
		{
			Database = DB;
		}
	}
}

#pragma region WeaponSubsystemTest
void UWeaponSubsystem::WeaponSubsystemTest(FWeaponCreateParams& Params, TMap<EWeaponCategory, UChildActorComponent*>& Inventory)
{
	Params.Type = EWeaponCategory::EBP_Sword;
	Params.Player->SetInventoryMaxSlots(2);

	UE_LOG(LogWeaponSubsystem, Warning, TEXT("- START OF WEAPON SUBSYSTEM TEST -"));
	if (SetWeapon(Params, Inventory) == EWeaponUpgradeStatus::E_Success)
	{
		UE_LOG(LogWeaponSubsystem, Warning, TEXT("Tree branch has been added."));
	}

	Params.DebugUpgrade = -1;
	Params.Type = EWeaponCategory::EBP_Sword;
	if (SetWeapon(Params, Inventory) == EWeaponUpgradeStatus::E_Success)
	{
		UE_LOG(LogWeaponSubsystem, Warning, TEXT("Tree branch has been upgraded to a surdy branch."));
	}

	Params.DebugUpgrade = -1;
	Params.Type = EWeaponCategory::EBP_Sword;
	if (SetWeapon(Params, Inventory) == EWeaponUpgradeStatus::E_Success)
	{
		UE_LOG(LogWeaponSubsystem, Warning, TEXT("Sturdy branch has been upgraded to a wooden sword."));
	}

	Params.DebugUpgrade = -1;
	Params.Type = EWeaponCategory::EBP_Sword;
	if (SetWeapon(Params, Inventory) == EWeaponUpgradeStatus::E_MaxUpgrade)
	{
		UE_LOG(LogWeaponSubsystem, Warning, TEXT("E_MAXUPGRADE: Wooden sword is the last upgrade, and cannot be upgraded any more."));
	}

	Params.DebugUpgrade = 1;
	Params.Type = EWeaponCategory::EBP_Sword;
	if (SetWeapon(Params, Inventory) == EWeaponUpgradeStatus::E_Success)
	{
		UE_LOG(LogWeaponSubsystem, Warning, TEXT("Wooden sword was downgraded into a sturdy branch (via debug upgrade)."));
	}

	Params.DebugUpgrade = 2;
	Params.Type = EWeaponCategory::EBP_Sword;
	if (SetWeapon(Params, Inventory) == EWeaponUpgradeStatus::E_Success)
	{
		UE_LOG(LogWeaponSubsystem, Warning, TEXT("Sturdy branch was upgraded into a wooden sword (via debug upgrade)."));
	}

	Params.DebugUpgrade = 0;
	Params.Type = EWeaponCategory::EBP_Sword;
	if (SetWeapon(Params, Inventory) == EWeaponUpgradeStatus::E_Success)
	{
		UE_LOG(LogWeaponSubsystem, Warning, TEXT("Wooden sword was downgraded into a tree branch (via debug upgrade)."));
	}

	Params.DebugUpgrade = -3;
	Params.Type = EWeaponCategory::EBP_Sword;
	if (SetWeapon(Params, Inventory) == EWeaponUpgradeStatus::E_Success)
	{
		UE_LOG(LogWeaponSubsystem, Warning, TEXT("Tree branch is the lowest upgrade type. Did not change the type (via debug upgrade)."));
	}

	Params.DebugUpgrade = 5;
	Params.Type = EWeaponCategory::EBP_Sword;
	if (SetWeapon(Params, Inventory) == EWeaponUpgradeStatus::E_Success)
	{
		UE_LOG(LogWeaponSubsystem, Warning, TEXT("Tree branch was upgraded into a wooden sword (via debug upgrade)."));
	}

	Params.DebugUpgrade = -1;
	Params.Type = EWeaponCategory::EBP_BowToGun;
	if (SetWeapon(Params, Inventory) == EWeaponUpgradeStatus::E_Success)
	{
		UE_LOG(LogWeaponSubsystem, Warning, TEXT("Bow and Arrow has been added."));
	}

	Params.DebugUpgrade = -1;
	Params.Type = EWeaponCategory::EBP_Axe;
	if (SetWeapon(Params, Inventory) == EWeaponUpgradeStatus::E_MaxSlots)
	{
		UE_LOG(LogWeaponSubsystem, Warning, TEXT("E_MAXSLOTS: Axe cannot be added."));
		UE_LOG(LogWeaponSubsystem, Warning, TEXT("- END OF WEAPON SUBSYSTEM TEST -"));
	}
}
#pragma endregion

bool UWeaponSubsystem::GetCategoryUpgrades(EWeaponCategory& Category, TArray<TSubclassOf<ABaseWeapon>>& Upgrades)
{
	switch (Category)
	{
	case EWeaponCategory::EBP_Sword:
		Upgrades = Database->SwordUpgrades;
		break;
	case EWeaponCategory::EBP_BowToGun:
		Upgrades = Database->BowToGunUpgrades;
		break;
	case EWeaponCategory::EBP_Axe:
		Upgrades = Database->AxeUpgrades;
		break;
	case EWeaponCategory::EBP_Spear:
		Upgrades = Database->SpearUpgrades;
		break;
	case EWeaponCategory::EBP_Gauntlets:
		Upgrades = Database->GauntletsUpgrades;
		break;
	case EWeaponCategory::EBP_FireStaff:
		Upgrades = Database->FireStaffUpgrades;
		break;
	case EWeaponCategory::EBP_IceStaff:
		Upgrades = Database->IceStaffUpgrades;
		break;
	case EWeaponCategory::EBP_LightningStaff:
		Upgrades = Database->LightningStaffUpgrades;
		break;
	case EWeaponCategory::EBP_EarthStaff:
		Upgrades = Database->EarthStaffUpgrades;
		break;
	default:
		return false;
	}

	return true;
}

int32 UWeaponSubsystem::GetCategoryUpgradeSize(EWeaponCategory& Category)
{
	TArray<TSubclassOf<ABaseWeapon>> Upgrades;
	if (GetCategoryUpgrades(Category, Upgrades))
	{
		return Upgrades.Num() - 1;
	}

	return -1;
}

EWeaponUpgradeStatus UWeaponSubsystem::SetWeapon(FWeaponCreateParams& Params, TMap<EWeaponCategory, UChildActorComponent*>& Inventory)
{
	// We will check if our parameter inputs are valid before proceeding.
	TArray<TSubclassOf<ABaseWeapon>> Upgrades;
	if (!GetCategoryUpgrades(Params.Type, Upgrades))
	{
		return EWeaponUpgradeStatus::E_Error;
	}

	if (!Params.Attachment.IsValid())
	{
		return EWeaponUpgradeStatus::E_Error;
	}

	if (!Params.Player.IsValid())
	{
		return EWeaponUpgradeStatus::E_Error;
	}

	// We search through the inventory for an existing weapon with the same EWeaponCategory.
	TWeakObjectPtr<UChildActorComponent> WC;
	for (auto It = Inventory.begin(); It != Inventory.end(); ++It)
	{
		if (It->Key == Params.Type)
		{
			WC = It->Value;
			break;
		}
	}


	if (WC.IsValid()) // We upgrade/modify the existing weapon if it exists
	{
		ABaseWeapon* W = Cast<ABaseWeapon>(WC->GetChildActor());

		// We'll check if the weapon can be upgraded further or if will be modified
		if (W && W->GetWeaponUpgrade() < Upgrades.Num() - 1 || Params.DebugUpgrade != -1)
		{
			return UpgradeWeapon(Params, Inventory, WC, Upgrades);
		}
		else // We cannot upgrade the weapon any further. Womp womp.
		{
			return EWeaponUpgradeStatus::E_MaxUpgrade;
		}
	}
	else if (Inventory.Num() < Params.Player->GetInventoryMaxSlots()) // Otherwise, we create one!
	{
		return CreateWeapon(Params, Inventory, Upgrades);
	}

	return EWeaponUpgradeStatus::E_MaxSlots; // No space is left for another weapon on the Inventory. Womp womp.
}

EWeaponUpgradeStatus UWeaponSubsystem::UpgradeWeapon(FWeaponCreateParams& Params, TMap<EWeaponCategory, UChildActorComponent*>& Inventory, TWeakObjectPtr<UChildActorComponent> WeaponComponent, TArray<TSubclassOf<ABaseWeapon>>& Upgrades)
{
	// We end all activity from the old weapon
	if (ABaseWeapon* W = Cast<ABaseWeapon>(WeaponComponent->GetChildActor()))
	{
		W->StopWeapon();

		Params.Type = W->GetEWeaponCategory();

		/*
		* DebugUpgrade is a variable that will store the value for the next upgrade availbale to the weapon.
		*
		* It gets its name from your ability to override the weapon upgrading system and allowing you to
		* choose which upgrade from the weapon type you'd like to pick.
		*
		* If DebugUpgrade is negative one     -> Automatically assigned to the next upgrade number.
		* If DebugUpgrade is any other number -> You pick the upgrade we will get.
		*/
		Params.DebugUpgrade = (Params.DebugUpgrade == -1) ? W->GetWeaponUpgrade() + 1 : Params.DebugUpgrade;

		Params.Player->OPL.Remove(W, TEXT("StopWeapon"));
	}

	WeaponComponent->DestroyChildActor();
	WeaponComponent->UnregisterComponent();

	Inventory.Remove(Params.Type);

	// ...and create its replacement!
	return CreateWeapon(Params, Inventory, Upgrades);
}

EWeaponUpgradeStatus UWeaponSubsystem::CreateWeapon(FWeaponCreateParams& Params, TMap<EWeaponCategory, UChildActorComponent*>& Inventory, TArray<TSubclassOf<ABaseWeapon>>& Upgrades)
{
	// We will check if our parameter inputs are valid before proceeding.
	if (Upgrades.IsEmpty())
	{
		return EWeaponUpgradeStatus::E_Error;
	}

	// We set DebugUpgrade to a number between zero and the size of the Upgrades .
	Params.DebugUpgrade = (Params.DebugUpgrade < 0) ? 0 : FMath::Min<int32>(Params.DebugUpgrade, Upgrades.Num() - 1);

	TSubclassOf<ABaseWeapon> Class = Upgrades[Params.DebugUpgrade];
	if (!IsValid(Class))
	{
		return EWeaponUpgradeStatus::E_Error;
	}

	if (!Params.Player.IsValid())
	{
		return EWeaponUpgradeStatus::E_Error;
	}

	// We initialize the weapon
	UChildActorComponent* WeaponComponent = NewObject<UChildActorComponent>(Params.Player.Get(), UChildActorComponent::StaticClass());
	Inventory.Add(Params.Type, WeaponComponent);

	// We register the correct weapon type and attach it to the player capsule
	WeaponComponent->RegisterComponent();
	WeaponComponent->AttachToComponent(Params.Attachment.Get(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false));
	WeaponComponent->SetChildActorClass(Class);
	WeaponComponent->AddLocalRotation(FRotator(0.f, -90.f, 0.f));

	// We can safely cast here since the Class was validated and strictly takes in ABaseWeapon class types only.
	ABaseWeapon* Weapon = Cast<ABaseWeapon>(WeaponComponent->GetChildActor());

	// We stop the weapon when the player dies
	Params.Player->OPL.AddDynamic(Weapon, &ABaseWeapon::StopWeapon);

	// Setup the weapon's parent
	Weapon->SetParent(Params.Player.Get());

	// We set up extra properties
	Weapon->SetWeaponUpgrade((Params.DebugUpgrade));
	Weapon->SetWeaponCategory(Params.Type); // UpgradeWeapon reads this back to find the inventory slot

	if (Weapon->AbsoluteRotation())
	{
		WeaponComponent->SetAbsolute(false, true, false);
	}

	return EWeaponUpgradeStatus::E_Success;
}