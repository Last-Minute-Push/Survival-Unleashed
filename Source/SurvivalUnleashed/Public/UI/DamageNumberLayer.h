#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageNumberLayer.generated.h"

class UCanvasPanel;
class UDamageNumbersUI;
class ASpawnDirector;
class AEnemyBase;

USTRUCT()
struct FDamageNumberEntry
{
	GENERATED_BODY()

	UPROPERTY() TObjectPtr<UDamageNumbersUI> Widget;
	FVector WorldLocation = FVector::ZeroVector;
	float Age = -1.f; // below 0 = not showing
};

// Full-screen layer that shows a floating number for every enemy hit. Reuses a fixed set of widgets.
UCLASS()
class SURVIVALUNLEASHED_API UDamageNumberLayer : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> NumberCanvas;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Numbers")
	TSubclassOf<UDamageNumbersUI> NumberClass;

	// Most numbers on screen at once; past this, the oldest is reused early
	UPROPERTY(EditDefaultsOnly, Category = "Damage Numbers", meta = (ClampMin = "1"))
	int32 MaxNumbers = 60;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Numbers", meta = (ClampMin = "0.1"))
	float Lifetime = 0.7f;

	// How far the number floats up over its lifetime (world units)
	UPROPERTY(EditDefaultsOnly, Category = "Damage Numbers")
	float RiseDistance = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Numbers")
	float HeightAboveEnemy = 120.f;

	// Random sideways offset so numbers from the same enemy don't stack exactly
	UPROPERTY(EditDefaultsOnly, Category = "Damage Numbers")
	float Jitter = 30.f;

private:
	UPROPERTY(Transient)
	TArray<FDamageNumberEntry> Entries;

	int32 NextEntry = 0;
	TWeakObjectPtr<ASpawnDirector> SpawnDirector;

	UFUNCTION()
	void HandleEnemyHit(AEnemyBase* Enemy, float Damage);

	void UpdateEntry(FDamageNumberEntry& Entry, APlayerController* PC);
};
