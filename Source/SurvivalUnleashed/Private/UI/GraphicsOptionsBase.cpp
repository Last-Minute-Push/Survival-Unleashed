#include "UI/GraphicsOptionsBase.h"
#include "UI/ButtonBase.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
	// Index = EWindowMode value (Fullscreen 0, WindowedFullscreen 1, Windowed 2)
	const TCHAR* WindowModeNames[] = { TEXT("Fullscreen"), TEXT("Borderless Window"), TEXT("Windowed") };

	// Index = scalability level (0 Low ... 3 Epic)
	const TCHAR* QualityNames[] = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
}

void UGraphicsOptionsBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	WindowModeCombo->ClearOptions();
	for (const TCHAR* Name : WindowModeNames) WindowModeCombo->AddOption(Name);

	QualityCombo->ClearOptions();
	for (const TCHAR* Name : QualityNames) QualityCombo->AddOption(Name);

	// Resolutions the monitor supports
	TArray<FIntPoint> Supported;
	if (!UKismetSystemLibrary::GetSupportedFullscreenResolutions(Supported) || Supported.Num() == 0)
	{
		UKismetSystemLibrary::GetConvenientWindowedResolutions(Supported);
	}

	Resolutions.Reset();
	ResolutionCombo->ClearOptions();
	for (const FIntPoint& Res : Supported)
	{
		if (Resolutions.Contains(Res)) continue; // same size at different refresh rates
		Resolutions.Add(Res);
		ResolutionCombo->AddOption(FString::Printf(TEXT("%d x %d"), Res.X, Res.Y));
	}

	WindowModeCombo->OnSelectionChanged.AddDynamic(this, &UGraphicsOptionsBase::WindowModeChanged);
	ApplyButton->CodeButtonClick.AddDynamic(this, &UGraphicsOptionsBase::ApplyPressed);
}

void UGraphicsOptionsBase::NativeConstruct()
{
	Super::NativeConstruct();
	LoadCurrentSettings(); // always show what's actually in use, even after backing out without applying
}

void UGraphicsOptionsBase::LoadCurrentSettings()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;

	WindowModeCombo->SetSelectedIndex(static_cast<int32>(Settings->GetFullscreenMode()));

	// The current resolution may not be in the monitor's list (a custom window size); add it so it can be shown
	const FIntPoint Current = Settings->GetScreenResolution();
	int32 ResIndex = Resolutions.IndexOfByKey(Current);
	if (ResIndex == INDEX_NONE)
	{
		ResIndex = Resolutions.Add(Current);
		ResolutionCombo->AddOption(FString::Printf(TEXT("%d x %d"), Current.X, Current.Y));
	}
	ResolutionCombo->SetSelectedIndex(ResIndex);

	// -1 means mixed/custom levels, 4 is Cinematic
	const int32 Quality = Settings->GetOverallScalabilityLevel();
	QualityCombo->SetSelectedIndex(Quality < 0 ? 2 : FMath::Min(Quality, 3));

	VSyncCheck->SetIsChecked(Settings->IsVSyncEnabled());

	UpdateResolutionEnabled();
}

void UGraphicsOptionsBase::WindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdateResolutionEnabled();
}

void UGraphicsOptionsBase::UpdateResolutionEnabled()
{
	// Borderless always uses the desktop resolution, so the choice would do nothing
	ResolutionCombo->SetIsEnabled(WindowModeCombo->GetSelectedIndex() != EWindowMode::WindowedFullscreen);
}

void UGraphicsOptionsBase::ApplyPressed()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;

	const int32 Mode = WindowModeCombo->GetSelectedIndex();
	if (Mode != INDEX_NONE)
	{
		Settings->SetFullscreenMode(EWindowMode::ConvertIntToWindowMode(Mode));
	}

	const int32 ResIndex = ResolutionCombo->GetSelectedIndex();
	if (Resolutions.IsValidIndex(ResIndex))
	{
		Settings->SetScreenResolution(Resolutions[ResIndex]);
	}

	const int32 Quality = QualityCombo->GetSelectedIndex();
	if (Quality != INDEX_NONE)
	{
		Settings->SetOverallScalabilityLevel(Quality);
	}

	Settings->SetVSyncEnabled(VSyncCheck->IsChecked());

	// Applies resolution, window mode and scalability, then writes GameUserSettings.ini
	Settings->ApplySettings(false);
}

