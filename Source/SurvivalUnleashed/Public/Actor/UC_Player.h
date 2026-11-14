// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "UI/PlayerHUDBase.h"
#include "UI/PauseMenuBase.h"
#include "Enemies/BossEventSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/PlayerStatsComponent.h"
#include "Utility/UpgradeOffers.h"
#include "GameFramework/CheatManagerDefines.h"
#include "UC_Player.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerLost);


class ABaseWeapon;
class ULevelUpScreen;
class UInputComponent;
class USphereComponent;
class UWeaponSubsystem;
class UCapsuleComponent;

UCLASS()
class SURVIVALUNLEASHED_API AUC_Player : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AUC_Player();

	virtual void Tick(float DeltaTime) override;


	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPauseMenuBase> PauseClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI")
	TObjectPtr<UPauseMenuBase> PauseHUD;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI")
	TObjectPtr<UPlayerHUDBase> PlayerHUD;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PC")
	TObjectPtr<APlayerController> APC;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PC")
	APlayerController* APCP;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI")
	TObjectPtr<UUserWidget> UUW;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	ULevelUpScreen* CurrentLvlScreen;
	UPROPERTY(VisibleAnywhere, BlueprintAssignable, BlueprintCallable, Category = "ReloadEnd")
	FOnPlayerLost OPL;

	void LevelUpPause();
	void LevelUpResume();
	void PauseGame();
	void ResumeGame();
	void PlayerLost();
	void MagnetEnable();

	UFUNCTION() inline FVector GetLocation() { return GetMesh()->GetRelativeLocation(); }

	UCapsuleComponent* GetCapsuleComponent();
	USphereComponent* GetRadiusComponent();

	UPlayerStatsComponent* GetPlayerStatsComp();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	inline float GetLifeStealHealAmount() const { return LifeStealHeal; }
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	inline int32 GetWeaponCount() const { return WeaponInventory.Num(); }
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	inline int32 GetInventoryMaxSlots() const { return InventoryMaxSlots; }
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	inline void SetInventoryMaxSlots(int32 Slots) { InventoryMaxSlots = Slots; }
	
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void StartWeapons();
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void SetWeapon(EWeaponCategory Type);
	bool isPaused;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> AttractedActors;
	UFUNCTION()
	void UpdateKeyBinds();
protected:
	UPROPERTY(Category = Character, VisibleAnywhere)
	class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, Category = Character)
	class UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, Category = Character)
	class USphereComponent* PickUpRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* Moves;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* Dashes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* Paused;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* OverlayNext;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* OverlayPrev;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* OverlayTrigger;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float LifeStealHeal = -2;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Weapons")
	UWeaponSubsystem* WeaponSubsystem;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons")
	TMap<EWeaponCategory, int32> StartingWeapons;
	UPROPERTY(VisibleAnywhere, Transient, Category = "Weapons")
	TMap<EWeaponCategory, UChildActorComponent*> WeaponInventory;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapons", meta = (ClampMin = "1"))
	uint8 InventoryMaxSlots;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	UPlayerStatsComponent* PlayerStatsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Boss")
	UBossEventSubsystem* BossEventSubsystem;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	FVector2D LastKnownInput;
	const FVector Forward = FVector::ForwardVector;
	const FVector Right = FVector::RightVector;
	float RegenTimer = 0;
	float DashCDTimer = 0;
	float DashlengthTimer = 0;
	float clearDirectionTimer = 0;
	float directionhold = 0.2f;
	bool IsDashing = false;
	bool CanDash = true;
	bool Magnet = false;
	float MagnetTimer = 0;


	// - Debug Overlay -
#if UE_WITH_CHEAT_MANAGER
	void NextOverlay(const FInputActionValue& Value);
	void PreviousOverlay(const FInputActionValue& Value);
	void TriggerOverlay(const FInputActionValue& Value);
#endif

	UFUNCTION() 
	void SetOwnedWeapons(TArray<FOwnedWeapon>& OwnedWeapons);
	void Move(const FInputActionValue& Value);
	void Dash(const FInputActionValue& Value);
	void PlayerPause(const FInputActionValue& Value);
	void SetPlayerSpeed(float value);
	void EnableCollision();
	void DisableCollision();
	UFUNCTION()
	void UpdatePickUpRadius(float value);
	UFUNCTION()
	void HandlePickUpOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult);
};
