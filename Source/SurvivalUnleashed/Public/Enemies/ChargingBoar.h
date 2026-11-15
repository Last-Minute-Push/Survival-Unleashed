// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "ChargingBoar.generated.h"

class UDecalComponent;
class UMaterialInstanceDynamic;

/**
 * Specialist (06:00) that teaches the player to watch for warnings.
 *  Approach: walks at the player (EnemyData Move Speed, 250 uu/s).
 *  Wind-up:  within 900 uu with a clear line, stops and paws for 1.0 s. A path decal shows exactly where
 *            the charge will go, cut short where it would hit an obstacle. The direction is locked now.
 *  Charge:   runs the locked direction at 1,200 uu/s for up to 1,500 uu. Can't turn. Rams the player once for 20.
 *  Stunned:  hitting an obstacle stuns it for 1.5 s (the normal Stun status, so it gets the stun tint).
 *  Recover:  4 s before it can wind up again; it walks at the player meanwhile.
 */
UCLASS()
class SURVIVALUNLEASHED_API AChargingBoar : public AEnemyBase
{
	GENERATED_BODY()

public:
	AChargingBoar();

	virtual void Tick(float DeltaTime) override;
	virtual void OnPoolAcquire_Implementation() override;
	virtual float GetContactDamage() const override;

protected:
	virtual void BeginPlay() override;
	virtual FVector GetDesiredDirection() const override;

	// A red strip showing the charge path. Needs a deferred decal material; an optional scalar "Progress"
	// (0 to 1) fills it along its length during the wind-up.
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UDecalComponent> PathDecal;

	UPROPERTY(EditDefaultsOnly, Category = "Boar|Charge", meta = (ClampMin = "0"))
	float ChargeTriggerRange = 900.f;

	UPROPERTY(EditDefaultsOnly, Category = "Boar|Charge", meta = (ClampMin = "0.01"))
	float WindupTime = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Boar|Charge", meta = (ClampMin = "0"))
	float ChargeSpeed = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Boar|Charge", meta = (ClampMin = "0"))
	float ChargeLength = 1500.f;

	// One hit per charge, in place of the normal contact damage while charging
	UPROPERTY(EditDefaultsOnly, Category = "Boar|Charge", meta = (ClampMin = "0"))
	float ChargeDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Boar|Charge", meta = (ClampMin = "0"))
	float StunTime = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Boar|Charge", meta = (ClampMin = "0"))
	float ChargeCooldown = 4.f;

	// Sent to the material's MoveAmount while charging, so the legs run (above 1 = a bigger stride than walking)
	UPROPERTY(EditDefaultsOnly, Category = "Boar|Charge", meta = (ClampMin = "0"))
	float ChargeMoveAmount = 2.f;

private:
	enum class EState : uint8
	{
		Approach,
		Windup,
		Charge,
		Stunned
	};

	EState State = EState::Approach;
	float StateTime = 0.f;
	float CooldownRemaining = 0.f;

	FVector ChargeDirection = FVector::ForwardVector;
	float ChargeRemaining = 0.f;
	bool bRammedThisCharge = false;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PathDecalMaterial;

	void SetState(EState NewState);

	bool CanStartCharge(const APawn* Player) const;
	void StartWindup(const APawn* Player);
	void TickCharge(APawn* Player, float DeltaTime);

	// How far the charge can go before an obstacle stops it (up to ChargeLength)
	float MeasureChargePath(const FVector& Direction) const;
	void ShowPath(float Length);
	void HidePath();
};
