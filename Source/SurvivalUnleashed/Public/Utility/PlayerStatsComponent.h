// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/WeaponDatabase.h"
#include "PlayerStatsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthUI, float, Current, float, Max);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHurt, float, Ratio);
DECLARE_DYNAMIC_DELEGATE(FOnHurtEffect);
DECLARE_DYNAMIC_DELEGATE(FOnHealEffect);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDeath, float, Ratio);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHeal, float, Ratio);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnEXPPickUp, float, Ratio, float,Max);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnLevelUp, float, Level, float, EXP);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnEXPRadiusUpdate, float, NewExpRadius);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDead);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerLevelUp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerFocusUp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerLevelUpEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerFocusUpEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponUpgrade, EWeaponCategory, Category);


UENUM(BlueprintType)
enum class EPlayerStats : uint8
{
	EPS_Attack,
	EPS_Health,
	EPS_Defense,
	EPS_WeaponAttack,
	EPS_AttackSpeed,
	EPS_Reflect,
	EPS_DashCooldown,
	EPS_DashLength,
	EPS_Regen,
	EPS_LifeSteal,
	EPS_EXPGain,
	EPS_Evasion,
	EPS_PickupRange,
	EPS_ProjectileAmount,
	EPS_ProjectileSpeed,
	EPS_HealAmount,
	EPS_Count UMETA(Hidden)
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatUIUpdate, EPlayerStats, PlayerStatsArray, int32, Stats);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVALUNLEASHED_API UPlayerStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerStatsComponent();

	UPROPERTY(VisibleAnywhere, Category = Hurt)
	FOnHurt OnHurt;
	UPROPERTY(VisibleAnywhere, Category = Death)
	FOnDeath OnDeath;
	UPROPERTY(VisibleAnywhere, Category = Death)
	FOnHeal OnHeal;
	UPROPERTY(VisibleAnywhere, Category = EXPRadUpdate)
	FOnEXPRadiusUpdate OnRadUpdate;
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = hit)
	FOnHit OnHitAn;
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = dead)
	FOnDead OnDeadAn;
	UPROPERTY(VisibleAnywhere, BlueprintAssignable, BlueprintCallable, Category = "LevelUp")
	FOnPlayerLevelUp OPLU;
	UPROPERTY(VisibleAnywhere, BlueprintAssignable, BlueprintCallable, Category = "FocusUp")
	FOnPlayerFocusUp OPFU;
	UPROPERTY(VisibleAnywhere, BlueprintAssignable, BlueprintCallable, Category = "LevelUpEnd")
	FOnPlayerLevelUpEnd OPLUE;
	UPROPERTY(VisibleAnywhere, BlueprintAssignable, BlueprintCallable, Category = "FocusUpEnd")
	FOnPlayerFocusUpEnd OPFUE;
	UPROPERTY(VisibleANywhere, BlueprintAssignable, BlueprintCallable, Category = "WeaponUpgrade")
	FOnWeaponUpgrade OnWeaponUpgrade;

	FOnHealEffect OnHealEffect;
	FOnHurtEffect OnHurtEffect;
	FOnHealthUI OnHealthUIEffect;
	FOnStatUIUpdate OnStatUI;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	class ULevelUpScreen* CurrentLvlScreen;

	FOnEXPPickUp OnEXPPickUp;

	FOnLevelUp OnLevelUp;
	enum ELevelUpStyle
	{
		ESP_Focus,
		ESP_Level
	};
	TArray<ELevelUpStyle> QueueStyle;

	void CallLevelUp();
	void CallFocusStat();

	
	TMap<EPlayerStats, int32> StatRanks;
	const TMap<EPlayerStats, int32>& GetStatRanks(); 
	void ApplyStatUpgrade(EPlayerStats Stat);
	void FinishUpgrade();
	TArray<EPlayerStats> MainStatArrayForUI;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
private:
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser); //Does a evasion check first!
	
	//PlayerStats, 0 = not enabled!

	//Flat values
	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float MaxHealth = 100;
	UPROPERTY()
	float CurrentHealth = 0;
	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float DashCooldown = 2;
	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float DashLength = 0.25;
	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float Regen = 0; // this is in seconds
	UPROPERTY()
	float ProjectileAmount = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float PickupRange = 160;
	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float MoveSpeed = 800;

	//% Chance stats
	UPROPERTY(EditDefaultsOnly)
	float LifeSteal = 0;
	UPROPERTY(EditDefaultsOnly)
	float Evasion = 0;

	//% Amount stats
	UPROPERTY(EditDefaultsOnly)
	float Attack = 1;
	UPROPERTY(EditDefaultsOnly)
	float WeaponAttack = 1;
	UPROPERTY(EditDefaultsOnly)
	float Defense = 1;
	UPROPERTY(EditDefaultsOnly)
	float AttackSpeed = 1;
	UPROPERTY(EditDefaultsOnly)
	float ProjectileSpeed = 1;
	UPROPERTY(EditDefaultsOnly)
	float HealAmount = 0;
	UPROPERTY(EditDefaultsOnly)
	float EXPGain = 0;
	UPROPERTY(EditDefaultsOnly)
	float Reflect = 0;

	//Focus Stats part of % amount stats but is a flat value for simple upgrade purposes
	UPROPERTY(EditDefaultsOnly)
	float Intelligence = 0;
	UPROPERTY(EditDefaultsOnly)
	float Barbaric = 0;
	UPROPERTY(EditDefaultsOnly)
	float Modern = 0;

	//Levels
	UPROPERTY(EditDefaultsOnly)
	int Level = 1;
	UPROPERTY(EditDefaultsOnly)
	int ExpReuirement = 100;
	UPROPERTY(EditDefaultsOnly)
	int ExpReuirementScaler = 50; //what to scale the exp requirement by
	UPROPERTY(EditDefaultsOnly)
	int CurrentExpCount = 0;
	UPROPERTY(EditDefaultsOnly)
	int OldExpRequirement = 0;
	

public:	

	void UpdateExpANDLevel(int amount); 
	void LevelUpOverall(); //happens on every levelup and is to give player an overall lvl up.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser); //applys taken damage
#pragma region StatsGettersAndSetters
	float GetMaxHealth();
	void SetMaxHealth(float health);
	UFUNCTION()
	void LevelUpMaxHealth();//Use to lvl up for player
	void LevelUpMaxHealthMinor();//Used for overall lvl up

	float GetCurrentHealth();
	void SetCurrentHealth(float health);

	float GetAttack(); // stat flat value is the % amount!
	void SetAttack(float attack); // Set flat value ex: 1.2 for 20% more attack
	UFUNCTION()
	void LevelUpAttack(); // Use to lvl up for player
	void LevelUpAttackMinor(); //Used for overall lvl up

	float GetDashCooldown();
	void SetDashCooldown(float CD); //in seconds!
	UFUNCTION()
	void LevelUpDashCooldown();

	float GetDashLength();
	void SetDashingLength(float DL); //in seconds! 
	UFUNCTION()
	void LevelUpDashLength();

	float GetRegen();
	void SetRegen(float regencd); // in seconds!
	UFUNCTION()
	void LevelUpRegen();

	float GetProjectileAmount();
	void SetProjectileAmount(float amount); //flat value of amount
	UFUNCTION()
	void LevelUpProjectileAmount();

	float GetProjectileSpeed(); // stat flat value is the % amount!
	void SetProjectileSpeed(float); // set flat value ex: 1.2 for 20% more speed
	UFUNCTION()
	void LevelUpProjectileSpeed();

	float GetPickupRange(); 
	void SetPickupRange(float value);
	UFUNCTION()
	void LevelUpPickupRange();

	float GetMoveSpeed();
	void SetMoveSpeeds(float value);

	float GetReflectStat(); //get % amount as float EX: 0.45 = 45% damage reflected back
	float GetReflectStatCalculated(); //adds the reflect dmg + attack + defense
	void SetReflectStat(float value); //set % amount , between 0-1 as a float.
	UFUNCTION()
	void LevelUpReflectStat(); // Use to lvl up for player

	float GetLifeStealStat(); //get % chance as float
	void SetLifeStealStat(float value); //set % chance, between 0-1 as a float.
	UFUNCTION()
	void LevelUpLifeStealStat();

	float GetEvasion();//get % chance as float
	void SetEvasion(float value); //set % chance, between 0-1 as a float.
	UFUNCTION()
	void LevelUpEvasion();

	float GetWeaponAttack(); // stat flat value, is the % amount!
	void SetWeaponAttack(float value); // Set flat value ex: 1.2 for 20% more attack
	UFUNCTION()
	void LevelUpWeaponAttack();
	 
	int GetDefense(); // stat flat value
	float GetDefensePercent(); //actual percent amount (USE FOR EVERYTHING!!)
	void SetDefense(int value); // Set flat value ex: 10 for 9% dmg reduction (not an accurate conversion but ya get the gist)
	UFUNCTION()
	void LevelUpDefense(); // Use to lvl up for player
	void LevelUpDefenseMinor(); //Used for overall lvl up

	float GetAttackSpeed(); // stat flat value, is the % amount!
	void SetAttackSpeed(float value); // Set flat value ex: 1.2 for 20% more attackspeed
	UFUNCTION()
	void LevelUpAttackSpeed();

	float GetHealAmount(); // stat flat value, is the % amount!
	void SetHealAmount(float value);  // Set flat value ex: 1.2 for 20% more Healing
	UFUNCTION()
	void LevelUpHealAmount();

	float GetEXPGain(); // stat flat value, is the % amount!
	void SetEXPGain(float value);	// Set flat value ex: 1.2 for 20% more exp
	UFUNCTION()
	void LevelUpEXPGain();

	float GetIntelligence(); // stat flat value!
	float GetIntelligencePercent();  //use for everything, is the bonus % gained from this stat!
	void SetIntelligence(float value);	 // Set flat value
	UFUNCTION()
	void LevelUpIntelligence();

	float GetBarbaric(); // stat flat value!
	float GetBarbaricPercent(); //use for everything, is the bonus % gained from this stat!
	void SetBarbaric(float value);	 // Set flat value 
	UFUNCTION()
	void LevelUpBarbaric();

	float GetModern(); // stat flat value!
	float GetModernPercent(); //use for everything, is the bonus % gained from this stat!
	void SetModern(float value);  // Set flat value
	UFUNCTION()
	void LevelUpModern();

	float GetCurrLevel();
	void SetCurrLevel(int value);

	int GetCurrentExpCount();
	int GetCurrentExpCountForDisplay();//use this for displaying on UI!
	void SetCurrentExpCount(int value);

	int GetExpReuirement();
	int GetExpReuirementForDisplay();//use this for displaying on UI!
	void SetExpReuirement(int value);

	int GetExpReuirementScaler();
	void SetExpReuirementScaler(int value);

#pragma endregion
	float TruncateToHundredths(float value);
};
