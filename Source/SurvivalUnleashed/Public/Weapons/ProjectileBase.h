// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Framework/Poolable.h"
#include "ProjectileBase.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class AProjectileBase;

// Called once per enemy hit. Single-cast and reassigned on every launch, so a reused
// projectile never keeps a binding from the weapon that fired it last time.
DECLARE_DELEGATE_TwoParams(FOnProjectileHit, AProjectileBase* /*Projectile*/, AActor* /*Target*/);

USTRUCT(BlueprintType)
struct FProjectileLaunchParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FVector Origin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FVector Direction = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (ClampMin = "0"))
	float Speed = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (ClampMin = "0"))
	float Range = 1500.f;

	// Extra enemies it can pass through: 0 = stops at the first hit, -1 = unlimited
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (ClampMin = "-1"))
	int32 Pierce = 0;
};

UCLASS(Abstract)
class SURVIVALUNLEASHED_API AProjectileBase : public AActor, public IPoolable
{
	GENERATED_BODY()

public:
	AProjectileBase();

	// Acquire a projectile from the pool and launch it. Returns nullptr if it couldn't.
	static AProjectileBase* Fire(UWorld* World, TSubclassOf<AProjectileBase> ProjectileClass, AActor* OwningWeapon,
		const FProjectileLaunchParams& Params, FOnProjectileHit OnHit);

	// Spawn this class's PrewarmCount up front so the first volley doesn't hitch
	static void Prewarm(UWorld* World, TSubclassOf<AProjectileBase> ProjectileClass);

	// Launches the projectile. Called by Fire() after acquiring from the pool.
	void Launch(AActor* OwningWeapon, const FProjectileLaunchParams& Params, FOnProjectileHit OnHit);

	virtual void OnPoolAcquire_Implementation() override;
	virtual void OnPoolRelease_Implementation() override;
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UCapsuleComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile", meta = (ClampMin = "0"))
	int32 PrewarmCount = 50;

private:
	UFUNCTION()
	void OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void HandleHit(AActor* Target);
	void ReleaseToPool();

	FOnProjectileHit HitCallback;
	TArray<TWeakObjectPtr<AActor>> AlreadyHit;

	FVector Direction = FVector::ForwardVector;
	float Speed = 0.f;
	float RangeRemaining = 0.f;
	int32 HitsRemaining = 0;   // INDEX_NONE = unlimited

	bool bActive = false;      // launched and not yet released
	bool bSpent = false;       // out of range or pierces; released at the end of Tick
};
