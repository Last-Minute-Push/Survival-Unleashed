// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/BaseWeapon.h"
#include "Weapons/WeaponData.h"
#include "Engine/DamageEvents.h"
#include "Enemies/EnemyBase.h"
#include "Actor/UC_Player.h"
#include "Utility/PlayerStatsComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogBaseWeapon, Log, All);

// Sets default values
ABaseWeapon::ABaseWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Pivot = CreateDefaultSubobject<USceneComponent>("Pivot");
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	
	SetRootComponent(Pivot);
	Mesh->SetupAttachment(Pivot);

	bAbsoluteRotation = false;
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseWeapon::SetParent(AUC_Player* P)
{
	Parent = P;
	ParentStats = P->GetPlayerStatsComp();
}

/*static*/float ABaseWeapon::ComputeWeaponHit(UWeaponData* W, UPlayerStatsComponent* P/*, UEnemyStats* E*/)
{
	return W ? ComputeHitFromBase(W->Damage, P) : 0.f;
	
}

/*static*/float ABaseWeapon::ComputeHitFromBase(float BaseDamage, UPlayerStatsComponent * P)
{
	if (P/*&& E*/)
	{
		// Attack and Weapon Attack are multipliers (1.0 = no bonus, 1.05 = +5%), and both already include their focus bonuses
		float damage = BaseDamage * P->GetAttack() * P->GetWeaponAttack();
		//if (e.IsElite and e.Modifier == Armored) damage *= 0.5;
		return damage;
	}

	return 0.f;
}

float ABaseWeapon::GetHitDamage() const
{
	return ComputeWeaponHit(Data, ParentStats);
}

void ABaseWeapon::StopWeapon()
{
	SetActorTickEnabled(false);
}

void ABaseWeapon::ApplyDamage(AActor* Source, AActor* Target)
{
	if (AEnemyBase* E = Cast<AEnemyBase>(Target))
	{
		//if (UEnemyData* EnemyData = E->GetEnemyData()) // For Elite and Armored enemy states - unused for now
        //{
		float Damage = GetHitDamage();

     		TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
	    	FDamageEvent DamageEvent(ValidDamageTypeClass);

			if (ParentStats->GetLifeStealStat() > 0.0f)
			{
				float chance = FMath::FRand();
				if (ParentStats->GetLifeStealStat() > chance)
				{
					if (float HealAmount = ParentStats->GetHealAmount() > 0.0f)
					{
						Parent->TakeDamage(Parent->GetLifeStealHealAmount() * HealAmount, DamageEvent, GetInstigatorController(), this);
					}
					else
					{
						Parent->TakeDamage(Parent->GetLifeStealHealAmount(), DamageEvent, GetInstigatorController(), this);
					}
				}
			}

	    	Target->TakeDamage(Damage, DamageEvent, GetInstigatorController(), Source);
	    	E->ApplyKnockback(Parent->GetActorLocation(), Data ? Data->Knockback : 0.f);

	    	OnWeaponHit.Broadcast(Damage);
		//}
	}
}
