// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/PlayerStatsComponent.h"
#include "Actor/UC_Player.h"
#include "Kismet/GameplayStatics.h"
#include "UI/LevelUpScreen.h"
#include "Math/UnrealMathUtility.h"

// Sets default values for this component's properties
UPlayerStatsComponent::UPlayerStatsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	
}


void UPlayerStatsComponent::CallLevelUp()
{
	if (OPLU.IsBound()) {
		OPLU.Broadcast();
	}
}

void UPlayerStatsComponent::CallFocusStat()
{
	if (OPFU.IsBound()) {
		OPFU.Broadcast();
	}
}

const TMap<EPlayerStats, int32>& UPlayerStatsComponent::GetStatRanks()
{
	return StatRanks;
}

void UPlayerStatsComponent::ApplyStatUpgrade(EPlayerStats Stat)
{
	int32& Rank = StatRanks.FindOrAdd(Stat);
	if (Rank == 0)
	{
		MainStatArrayForUI.Add(Stat);
		
	}
	++Rank;
	OnStatUI.Broadcast(Stat, Rank);
	switch (Stat)
	{
	case EPlayerStats::EPS_Attack:LevelUpAttack();break;
	case EPlayerStats::EPS_Health:LevelUpMaxHealth();break;
	case EPlayerStats::EPS_Defense:LevelUpDefense();break;
	case EPlayerStats::EPS_WeaponAttack:LevelUpWeaponAttack();break;
	case EPlayerStats::EPS_AttackSpeed:LevelUpAttackSpeed();break;
	case EPlayerStats::EPS_Reflect:LevelUpReflectStat();break;
	case EPlayerStats::EPS_DashCooldown:LevelUpDashCooldown();break;
	case EPlayerStats::EPS_DashLength:LevelUpDashLength();break;
	case EPlayerStats::EPS_Regen:LevelUpRegen();break;
	case EPlayerStats::EPS_LifeSteal:LevelUpLifeStealStat();break;
	case EPlayerStats::EPS_EXPGain:LevelUpEXPGain();break;
	case EPlayerStats::EPS_Evasion:LevelUpEvasion();break;
	case EPlayerStats::EPS_PickupRange:LevelUpPickupRange();break;
	case EPlayerStats::EPS_ProjectileAmount:LevelUpProjectileAmount();break;
	case EPlayerStats::EPS_ProjectileSpeed:LevelUpProjectileSpeed();break;
	case EPlayerStats::EPS_HealAmount:LevelUpHealAmount();break;
	}
	

	FinishUpgrade();
}

void UPlayerStatsComponent::FinishUpgrade()
{
	if (OPLUE.IsBound()) {
		OPLUE.Broadcast();
	}
}

// Called when the game starts
void UPlayerStatsComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	
	AActor* MyOwner = GetOwner();
	if (MyOwner) {
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &UPlayerStatsComponent::HandleTakeAnyDamage);
	}
}

void UPlayerStatsComponent::UpdateExpANDLevel(int amount)
{
	if (GetEXPGain() != 0) {
		CurrentExpCount += (amount * GetEXPGain());//add to current count
		OnEXPPickUp.Execute(GetCurrentExpCountForDisplay(), GetExpReuirementForDisplay());
	}
	else {
		CurrentExpCount += amount;
		OnEXPPickUp.Execute(GetCurrentExpCountForDisplay(), GetExpReuirementForDisplay());
	}
	

	while (CurrentExpCount > ExpReuirement) { //levelup the needed amount of times
		OnEXPPickUp.Execute(GetCurrentExpCountForDisplay(), 0);
		Level++; //level increase
		LevelUpOverall();

		if (Level % 5 == 0) {
			QueueStyle.Add(ELevelUpStyle::ESP_Focus);
		}
		else {
			QueueStyle.Add(ELevelUpStyle::ESP_Level);
		}
		OldExpRequirement = ExpReuirement;
		ExpReuirement += (ExpReuirementScaler + (Level*100)); 
	}
	OnLevelUp.Execute(Level, GetCurrentExpCountForDisplay());
	
}

void UPlayerStatsComponent::LevelUpOverall()
{
	LevelUpMaxHealthMinor();
	LevelUpAttackMinor();
	LevelUpDefenseMinor();
}

// Called every frame
void UPlayerStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerStatsComponent::TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	//Just Take Damage and apply anything needed.
	CurrentHealth = FMath::Clamp(CurrentHealth - (Damage * GetDefensePercent()), 0.0, MaxHealth);
	if (GetReflectStat() > 0) {
		if (DamageCauser) {
			UGameplayStatics::ApplyDamage(DamageCauser, Damage * GetReflectStatCalculated(), nullptr, DamagedActor, nullptr);
		}
	}
	if (CurrentHealth > 0) {
		if (Damage > 0) {
			if (OnHitAn.IsBound()) {
				OnHitAn.Broadcast();
			}
			if (IsValid(DamagedActor)) {
				if (OnHurt.IsBound() && OnHurt.GetUObject() != nullptr) {
					OnHurt.Execute(CurrentHealth / MaxHealth);
					//UE_LOG(Game, Log, TEXT("Lost Health"));
					//UE_LOG(Game, Log, TEXT("CurrentHealth is: %s"), *FString(UTF8_TO_TCHAR(std::to_string(CurrentHealth).c_str())));
					OnHurtEffect.Execute();
				}
			}
		}
		else {
			if (IsValid(DamagedActor)) {
				if (OnHeal.IsBound() && OnHeal.GetUObject() != nullptr) {
					OnHeal.Execute(CurrentHealth / MaxHealth);
					//UE_LOG(Game, Log, TEXT("Lost Health"));
					//UE_LOG(Game, Log, TEXT("CurrentHealth is: %s"), *FString(UTF8_TO_TCHAR(std::to_string(CurrentHealth).c_str())));
					OnHealEffect.Execute();
				}
			}
		}

	}
	else {
		if (OnDeadAn.IsBound()) {
			OnDeadAn.Broadcast();
		}
		DamagedActor->SetActorEnableCollision(false);
		APawn* OwningPawn = Cast<APawn>(GetOwner());
		if (OwningPawn) {
			//player lose stuff
			APlayerController* APlC = Cast<APlayerController>(OwningPawn->GetController());
			if (APlC) {
				AUC_Player* PC = Cast<AUC_Player>(OwningPawn);
				if (PC) {
					PC->OPL.Broadcast();
				}
				GetOwner()->DisableInput(APlC);
			}
		}

		if (IsValid(DamagedActor)) {
			if (OnDeath.IsBound() && OnDeath.GetUObject() != nullptr) {
				OnDeath.Execute(0);
				//UE_LOG(Game, Log, TEXT("Dead!"));

			}
			else {
				AActor* MyOwner = GetOwner();
				if (MyOwner) {
					MyOwner->OnTakeAnyDamage.RemoveDynamic(this, &UPlayerStatsComponent::HandleTakeAnyDamage);
				}
			}
		}
	}
}


void UPlayerStatsComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	//EVASION CHECKFIRST!!!
	if (GetEvasion() > 0) { //Got Evade stat?
		float chance = FMath::FRand();
		if (GetEvasion() < chance) { //if you fail to evade take damage!
			TakeDamage(DamagedActor, Damage, DamageType, InstigatedBy, DamageCauser);
		}
		else {
			// you dodged!
		}
	}
	else { //just take damage normally
		TakeDamage(DamagedActor, Damage, DamageType, InstigatedBy, DamageCauser);
	}
	
}

#pragma region StatsGettersAndSetters 

float UPlayerStatsComponent::GetMaxHealth()
{
	return MaxHealth + (MaxHealth * GetIntelligencePercent()) + (MaxHealth * GetModernPercent());
}
void UPlayerStatsComponent::SetMaxHealth(float health)
{
	MaxHealth = health;
}
void UPlayerStatsComponent::LevelUpMaxHealth()
{
	SetMaxHealth(MaxHealth + 10.0f);
	SetCurrentHealth(CurrentHealth + 10.0f);
	if (OnHeal.IsBound() && OnHeal.GetUObject() != nullptr) {
		OnHeal.Execute(CurrentHealth / MaxHealth);
		//UE_LOG(Game, Log, TEXT("Lost Health"));
		//UE_LOG(Game, Log, TEXT("CurrentHealth is: %s"), *FString(UTF8_TO_TCHAR(std::to_string(CurrentHealth).c_str())));
	}
}
void UPlayerStatsComponent::LevelUpMaxHealthMinor()
{
	SetMaxHealth(MaxHealth + 5.0f);
	SetCurrentHealth(CurrentHealth + 5.0f);
	if (OnHeal.IsBound() && OnHeal.GetUObject() != nullptr) {
		OnHeal.Execute(CurrentHealth / MaxHealth);
		//UE_LOG(Game, Log, TEXT("Lost Health"));
		//UE_LOG(Game, Log, TEXT("CurrentHealth is: %s"), *FString(UTF8_TO_TCHAR(std::to_string(CurrentHealth).c_str())));
	}
}
float UPlayerStatsComponent::GetCurrentHealth()
{
	return CurrentHealth;
}
void UPlayerStatsComponent::SetCurrentHealth(float health)
{
	CurrentHealth = health;
}
float UPlayerStatsComponent::GetAttack()
{
	return Attack + (Attack * GetIntelligencePercent()) + (Attack * GetBarbaricPercent());
}
void UPlayerStatsComponent::SetAttack(float attack)
{
	Attack = attack;
}
void UPlayerStatsComponent::LevelUpAttack()
{
	SetAttack(Attack + 0.05f);
}
void UPlayerStatsComponent::LevelUpAttackMinor()
{
	SetAttack(Attack + 0.025f);
}
float UPlayerStatsComponent::GetDashCooldown()
{
	return DashCooldown - (GetIntelligencePercent() * 0.1f);
}
void UPlayerStatsComponent::SetDashCooldown(float CD)
{
	DashCooldown = CD;
}
void UPlayerStatsComponent::LevelUpDashCooldown()
{
	SetDashCooldown(DashCooldown - (DashCooldown * 0.1f));
}
float UPlayerStatsComponent::GetDashLength()
{
	return DashLength + (DashLength * (GetIntelligencePercent() * 0.5f));
}
void UPlayerStatsComponent::SetDashingLength(float DL)
{
	DashLength = DL;
}
void UPlayerStatsComponent::LevelUpDashLength()
{
	SetDashingLength(DashLength + (DashLength * 0.2f));
}
float UPlayerStatsComponent::GetRegen()
{
	return Regen - (Regen *(GetBarbaricPercent() * 0.1f));
}
void UPlayerStatsComponent::SetRegen(float regencd)
{
	Regen = regencd;
}

void UPlayerStatsComponent::LevelUpRegen()
{
	if (Regen == 0) {
		SetRegen(2);
		SetRegen(Regen - (Regen * 0.1f));
	}
	else {
		SetRegen(Regen - (Regen * 0.1f));
	}
}

float UPlayerStatsComponent::GetProjectileAmount()
{
	return ProjectileAmount + GetIntelligencePercent();
}

void UPlayerStatsComponent::SetProjectileAmount(float amount)
{
	ProjectileAmount = amount;
}

void UPlayerStatsComponent::LevelUpProjectileAmount()
{
	SetProjectileAmount(ProjectileAmount + 1.0f);
}

float UPlayerStatsComponent::GetProjectileSpeed()
{
	return ProjectileSpeed + GetIntelligencePercent();
}

void UPlayerStatsComponent::SetProjectileSpeed(float value)
{
	ProjectileSpeed = value;
}

void UPlayerStatsComponent::LevelUpProjectileSpeed()
{
	SetProjectileSpeed(ProjectileSpeed + 0.1f);

}

float UPlayerStatsComponent::GetPickupRange()
{
	return PickupRange;
}

void UPlayerStatsComponent::SetPickupRange(float value)
{
	PickupRange = value;
}

void UPlayerStatsComponent::LevelUpPickupRange()
{
	SetPickupRange(PickupRange + 2.5f);
	if (OnRadUpdate.IsBound() && OnRadUpdate.GetUObject() != nullptr) {
		OnRadUpdate.Execute(GetPickupRange());
	}
}

float UPlayerStatsComponent::GetMoveSpeed()
{
	return MoveSpeed;
}

void UPlayerStatsComponent::SetMoveSpeeds(float value)
{
	MoveSpeed = value;
}

float UPlayerStatsComponent::GetReflectStat()
{
	return Reflect + (Reflect *(GetBarbaricPercent()/2.0f));
}
float UPlayerStatsComponent::GetReflectStatCalculated()
{
	return Reflect * ( ((1.0f - GetAttack())/2.0f)+ 1.0f)  *  (1.0f +( GetDefensePercent()/3.0f));
}

void UPlayerStatsComponent::SetReflectStat(float value)
{
	Reflect = value;
}

void UPlayerStatsComponent::LevelUpReflectStat()
{
	if (Reflect == 0) {
		SetReflectStat(0.15f);
	}
	else {
		SetReflectStat(Reflect + 0.05f);
	}
}


float UPlayerStatsComponent::GetLifeStealStat()
{
	return LifeSteal + (LifeSteal * (GetBarbaricPercent() / 3.0f));
}

void UPlayerStatsComponent::SetLifeStealStat(float value)
{
	LifeSteal = value;
}

void UPlayerStatsComponent::LevelUpLifeStealStat()
{
	if (LifeSteal == 0) {
		SetLifeStealStat(0.02f);
	}
	else {
		SetLifeStealStat(FMath::Clamp(LifeSteal + 0.04f,0.0f,1.0f));
	}
}

float UPlayerStatsComponent::GetEvasion()
{
	return Evasion + (Evasion* (GetIntelligencePercent()/2.0f));
}

void UPlayerStatsComponent::SetEvasion(float value)
{
	Evasion = value;
}

void UPlayerStatsComponent::LevelUpEvasion()
{
	if (Evasion == 0) {
		SetEvasion(0.1f);
	}
	else {
		SetEvasion(FMath::Clamp(Evasion + (Evasion * 0.1f), 0.0f, 1.0f));
	}
}

float UPlayerStatsComponent::GetWeaponAttack()
{
	return WeaponAttack + (WeaponAttack * GetModernPercent());
}

void UPlayerStatsComponent::SetWeaponAttack(float value)
{
	WeaponAttack = value;
}

void UPlayerStatsComponent::LevelUpWeaponAttack()
{
	SetWeaponAttack(WeaponAttack + 0.05f);
}

int UPlayerStatsComponent::GetDefense()
{
	return Defense;
}

float UPlayerStatsComponent::GetDefensePercent()
{
	return TruncateToHundredths((FMath::Loge(Defense + (Defense * GetBarbaricPercent()) + (Defense * GetModernPercent()))/FMath::Loge(50000.0f)));
}

void UPlayerStatsComponent::SetDefense(int value)
{
	Defense = value;
}

void UPlayerStatsComponent::LevelUpDefense()
{
	SetDefense(Defense + 5.0f);
}

void UPlayerStatsComponent::LevelUpDefenseMinor()
{
	SetDefense(Defense + 1.0f);
}

float UPlayerStatsComponent::GetAttackSpeed()
{
	return AttackSpeed + (AttackSpeed * (GetIntelligencePercent() /1.5f)) + (AttackSpeed * (GetModernPercent() / 1.5f));
}

void UPlayerStatsComponent::SetAttackSpeed(float value)
{
	AttackSpeed = value;
}

void UPlayerStatsComponent::LevelUpAttackSpeed()
{
	SetAttackSpeed(AttackSpeed + 0.05f);
}

float UPlayerStatsComponent::GetHealAmount()
{
	return HealAmount + (HealAmount * GetModernPercent());
}

void UPlayerStatsComponent::SetHealAmount(float value)
{
	HealAmount = value;
}

void UPlayerStatsComponent::LevelUpHealAmount()
{
	if (HealAmount == 0) {
		SetHealAmount(HealAmount + 1.1f);
	}
	else {
		SetHealAmount(HealAmount + 0.1f);
	}
}

float UPlayerStatsComponent::GetEXPGain()
{
	return EXPGain;
}

void UPlayerStatsComponent::SetEXPGain(float value)
{
	EXPGain = value;
}

void UPlayerStatsComponent::LevelUpEXPGain()
{
	if (EXPGain == 0) {
		SetEXPGain(1.1f);
	}
	else {
		SetEXPGain(EXPGain + 0.1f);
	}
}

float UPlayerStatsComponent::GetIntelligence()
{
	return Intelligence;
}

float UPlayerStatsComponent::GetIntelligencePercent()
{
	return (Intelligence * 0.05f);
}

void UPlayerStatsComponent::SetIntelligence(float value)
{
	Intelligence = value;
}

void UPlayerStatsComponent::LevelUpIntelligence()
{
	SetIntelligence(GetIntelligence() + 1.0f);
	if (OPFUE.IsBound()) {
		OPFUE.Broadcast();
	}
}

float UPlayerStatsComponent::GetBarbaric()
{
	return Barbaric;
}

float UPlayerStatsComponent::GetBarbaricPercent()
{
	return (Barbaric * 0.05f);
}

void UPlayerStatsComponent::SetBarbaric(float value)
{
	Barbaric = value;
}

void UPlayerStatsComponent::LevelUpBarbaric()
{
	SetBarbaric(GetBarbaric() + 1.0f);
	if (OPFUE.IsBound()) {
		OPFUE.Broadcast();
	}
}

float UPlayerStatsComponent::GetModern()
{
	return Modern;
}

float UPlayerStatsComponent::GetModernPercent()
{
	return (Modern * 0.05f); 
}

void UPlayerStatsComponent::SetModern(float value)
{
	Modern = value;
}

void UPlayerStatsComponent::LevelUpModern()
{
	SetModern(GetModern() + 1.0f);
	if (OPFUE.IsBound()) {
		OPFUE.Broadcast();
	}
}

float UPlayerStatsComponent::GetCurrLevel()
{
	return Level;
}

void UPlayerStatsComponent::SetCurrLevel(int value)
{
	Level = value;
}

int UPlayerStatsComponent::GetCurrentExpCount()
{
	return CurrentExpCount;
}

int UPlayerStatsComponent::GetCurrentExpCountForDisplay()
{
	return CurrentExpCount - OldExpRequirement;
}

void UPlayerStatsComponent::SetCurrentExpCount(int value)
{
	CurrentExpCount = value;
}

int UPlayerStatsComponent::GetExpReuirement()
{
	return ExpReuirement;
}

int UPlayerStatsComponent::GetExpReuirementForDisplay()
{
	return ExpReuirement - OldExpRequirement;
}

void UPlayerStatsComponent::SetExpReuirement(int value)
{
	ExpReuirement = value;
}

int UPlayerStatsComponent::GetExpReuirementScaler()
{
	return ExpReuirementScaler;
}

void UPlayerStatsComponent::SetExpReuirementScaler(int value)
{
	ExpReuirementScaler = value;
}

float UPlayerStatsComponent::TruncateToHundredths(float value)
{
	float scaled = value * 100.0f;

	float truncated = FMath::TruncToInt(scaled);

	return truncated / 100.0f;
}

#pragma endregion