#include "UI/DamageNumberLayer.h"
#include "UI/DamageNumbersUI.h"
#include "Enemies/SpawnDirector.h"
#include "Enemies/EnemyBase.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogDamageNumbersLayout, Log, All);

void UDamageNumberLayer::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Entries.SetNum(MaxNumbers); // widgets are created the first time each entry is used

	for (TActorIterator<ASpawnDirector> It(GetWorld()); It; ++It)
	{
		SpawnDirector = *It;
		break;
	}
	if (!SpawnDirector.IsValid())
	{
		UE_LOG(LogDamageNumbersLayout, Warning, TEXT("No Spawn Director in the level; damage numbers are off."));
	}
}

void UDamageNumberLayer::NativeConstruct()
{
	Super::NativeConstruct();

	// Construct/Destruct run every time the widget is added/removed, so binding here and
	// unbinding in Destruct keeps exactly one binding. AddUnique guards against doubles anyway.
	if (SpawnDirector.IsValid())
	{
		SpawnDirector->OnAnyEnemyHit.AddUniqueDynamic(this, &UDamageNumberLayer::HandleEnemyHit);
	}
}

void UDamageNumberLayer::NativeDestruct()
{
	if (SpawnDirector.IsValid())
	{
		SpawnDirector->OnAnyEnemyHit.RemoveDynamic(this, &UDamageNumberLayer::HandleEnemyHit);
	}
	Super::NativeDestruct();
}

void UDamageNumberLayer::HandleEnemyHit(AEnemyBase* Enemy, float Damage)
{
	if (!Enemy || !NumberClass || !NumberCanvas || Damage <= 0.f || Entries.Num() == 0)
	{
		return;
	}

	// Take the next entry in the ring; if it's still showing, it just restarts here
	FDamageNumberEntry& Entry = Entries[NextEntry];
	NextEntry = (NextEntry + 1) % Entries.Num();

	if (!Entry.Widget)
	{
		Entry.Widget = CreateWidget<UDamageNumbersUI>(this, NumberClass);
		UCanvasPanelSlot* CanvasSlot = NumberCanvas->AddChildToCanvas(Entry.Widget);
		CanvasSlot->SetAutoSize(true);
		CanvasSlot->SetAlignment(FVector2D(0.5f, 1.f)); // centered, sitting on the point
	}

	// Copy the location now: a killing hit releases the enemy to the pool right after this
	const FVector2D Offset = FMath::RandPointInCircle(Jitter);
	Entry.WorldLocation = Enemy->GetActorLocation() + FVector(Offset.X, Offset.Y, HeightAboveEnemy);
	Entry.Age = 0.f;

	Entry.Widget->SetDamage(Damage);
	Entry.Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
	UpdateEntry(Entry, GetOwningPlayer()); // place it now, so it never shows a frame at its old spot
}

void UDamageNumberLayer::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APlayerController* PC = GetOwningPlayer();
	for (FDamageNumberEntry& Entry : Entries)
	{
		if (Entry.Age < 0.f)
		{
			continue;
		}

		Entry.Age += InDeltaTime;
		if (Entry.Age >= Lifetime)
		{
			Entry.Age = -1.f;
			Entry.Widget->SetVisibility(ESlateVisibility::Collapsed);
			continue;
		}
		UpdateEntry(Entry, PC);
	}
}

void UDamageNumberLayer::UpdateEntry(FDamageNumberEntry& Entry, APlayerController* PC)
{
	if (!PC || !Entry.Widget)
	{
		return;
	}

	const float T = Entry.Age / Lifetime;

	// Re-project every frame so the number stays on its spot in the world as the camera follows the player
	FVector2D WidgetPos;
	const FVector World = Entry.WorldLocation + FVector(0.f, 0.f, RiseDistance * T);
	if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, World, WidgetPos, false))
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Entry.Widget->Slot))
		{
			CanvasSlot->SetPosition(WidgetPos);
		}
	}

	// Solid for most of its life, fading at the end
	Entry.Widget->SetRenderOpacity(1.f - T * T);
}
