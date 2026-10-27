#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h"
#include "GraphicsOptionsBase.generated.h"

class UComboBoxString;
class UCheckBox;
class UButtonBase;

/** Graphics tab of the Options menu. Reads and writes UGameUserSettings (saved to GameUserSettings.ini). */
UCLASS()
class SURVIVALUNLEASHED_API UGraphicsOptionsBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UComboBoxString> WindowModeCombo;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UComboBoxString> ResolutionCombo;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UComboBoxString> QualityCombo;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UCheckBox> VSyncCheck;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButtonBase> ApplyButton;

	UFUNCTION() void ApplyPressed();
	UFUNCTION() void WindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

private:
	// Same order as ResolutionCombo's options, so a selected index maps straight to a resolution
	TArray<FIntPoint> Resolutions;

	void LoadCurrentSettings();
	void UpdateResolutionEnabled();
};
