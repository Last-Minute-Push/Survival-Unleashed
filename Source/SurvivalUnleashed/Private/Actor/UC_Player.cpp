// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/UC_Player.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Weapons/BaseWeapon.h"
#include "Actor/EXP_PickUp.h"
#include "UI/LevelUpScreen.h"
#include "Weapons/WeaponSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Actor/Health_PickUp.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "Audio/Interfaces/AudioManagerInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlayer, Log, All);

// Sets default values
AUC_Player::AUC_Player()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bTickEvenWhenPaused = false;

	PlayerStatsComponent = CreateDefaultSubobject<UPlayerStatsComponent>("PlayerStatsComponent");
	this->GetCharacterMovement()->MaxWalkSpeed = PlayerStatsComponent->GetMoveSpeed();
	this->GetCharacterMovement()->bOrientRotationToMovement = true;
	this->GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->SetUsingAbsoluteRotation(true);
	SpringArm->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));
	SpringArm->TargetArmLength = 1200.0f;
	SpringArm->TargetOffset = FVector(0.0f, 0.0f, 90.0f);
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = false;
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;
	PickUpRadius = CreateDefaultSubobject<USphereComponent>("PickUpRadius");
	PickUpRadius->SetupAttachment(GetRootComponent());
	PickUpRadius->SetCollisionProfileName(FName("PickupRadius"));
	PickUpRadius->SetSphereRadius(PlayerStatsComponent->GetPickupRange());
	PickUpRadius->OnComponentBeginOverlap.AddDynamic(this, &AUC_Player::HandleEXPOverlap);
}

// Called when the game starts or when spawned
void AUC_Player::BeginPlay()
{
	Super::BeginPlay();
	APC = Cast<APlayerController>(GetController());
	APCP = Cast<APlayerController>(GetController());
	if (IsValid(APC)) {
		if (IsValid(HUDClass)) {
			UUW = CreateWidget(APC, HUDClass);
			PlayerHUD = Cast<UPlayerHUDBase>(UUW);
			PauseHUD = CreateWidget<UPauseMenuBase>(APC, PauseClass);
			if (PlayerHUD) {
				PlayerHUD->AddToViewport();
				PlayerHUD->SetHealth(PlayerStatsComponent->GetCurrentHealth());
				PlayerStatsComponent->OnHurt.BindDynamic(PlayerHUD, &UPlayerHUDBase::SetHealth);
				PlayerStatsComponent->OnHeal.BindDynamic(PlayerHUD, &UPlayerHUDBase::SetHealth);
				PlayerStatsComponent->OnHurtEffect.BindDynamic(PlayerHUD, &UPlayerHUDBase::PlayHurtFeedback);
				PlayerStatsComponent->OnHealEffect.BindDynamic(PlayerHUD, &UPlayerHUDBase::PlayHealFeedback);
				PlayerStatsComponent->OnDeath.BindDynamic(PlayerHUD, &UPlayerHUDBase::SetHealth);
				PlayerStatsComponent->OnEXPPickUp.BindDynamic(PlayerHUD, &UPlayerHUDBase::SetExperience);
				PlayerStatsComponent->OnLevelUp.BindDynamic(PlayerHUD, &UPlayerHUDBase::SetLevelAndExp);
				PlayerStatsComponent->OnRadUpdate.BindDynamic(this, &AUC_Player::UpdatePickUpRadius);

				PlayerStatsComponent->OnWeaponUpgrade.AddDynamic(this, &AUC_Player::SetWeapon);
				PlayerStatsComponent->OnStatUI.AddDynamic(PlayerHUD, &UPlayerHUDBase::SetStatsText);
				

				isPaused = false;

				if (!PauseHUD)
				{
					UE_LOG(LogPlayer, Error, TEXT("PauseMenu Creation Does not work"));
				}
			}
			else {
				//HUD NOT VALID
			}
		}
		else {
			//HUD CLASS NOT VALID			
		}
	}
	else {
		//PLAYER CONTROLLER NOT VALID
	}

	if (UWeaponSubsystem* SS = GetWorld()->GetSubsystem<UWeaponSubsystem>())
	{
		WeaponSubsystem = SS;
	}
	WeaponInventory.Reserve(InventoryMaxSlots);
	if (!StartingWeapons.IsEmpty())
	{
		FWeaponCreateParams Params;
		Params.Attachment = GetCapsuleComponent();
		Params.Player = this;

		for (auto It = StartingWeapons.begin(); It != StartingWeapons.end(); ++It)
		{
			Params.Type = It->Key;
			Params.DebugUpgrade = It->Value;
			WeaponSubsystem->SetWeapon(Params, WeaponInventory);
		}
	}
	else UE_LOG(LogPlayer, Warning, TEXT("No WeaponComponents have been initialized!\nPlayer will not have any weapons!"));

	/*- Uncomment to activate WeaponSubsystemTest -*/
	//FWeaponCreateParams Params;
	//Params.Attachment = GetCapsuleComponent();
	//Params.Player = this;
	//WeaponSubsystem->WeaponSubsystemTest(Params, WeaponInventory);

	CurrentLvlScreen->OnSetOwnedWeapons.AddDynamic(this, &AUC_Player::SetOwnedWeapons);

	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{

		if (I->GetLibrary()->BGM_GameplaySong) {
			I->PlayMusic(I->GetLibrary()->BGM_GameplaySong);
		}
	}
	else
	{
		UE_LOG(LogPlayer, Error, TEXT("No Audio Library Instance has been found!"));
	}

}

// Called every frame
void AUC_Player::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	


	for (int32 i = 0; i < AttractedActors.Num(); i++) {
		AActor* Actor = AttractedActors[i].Get();
		if (Actor) {
			FVector Direction = (this->GetActorLocation() - Actor->GetActorLocation()).GetSafeNormal();
			FVector Offset = Direction * 400.0f * DeltaTime;
			Actor->AddActorLocalOffset(Offset, false);
		}
	}
	if (GetPlayerStatsComp()->QueueStyle.Num() > 0) {
		if (GetPlayerStatsComp()->QueueStyle[0] == GetPlayerStatsComp()->ELevelUpStyle::ESP_Focus) {
			GetPlayerStatsComp()->CallFocusStat();
			GetPlayerStatsComp()->QueueStyle.RemoveAt(0);
		}
		else {
			GetPlayerStatsComp()->CallLevelUp();
			GetPlayerStatsComp()->QueueStyle.RemoveAt(0);
		}
	}
	if (GetPlayerStatsComp()->GetRegen() > 0) {
		RegenTimer += DeltaTime;
		if (RegenTimer >= GetPlayerStatsComp()->GetRegen()) {
			if (GetPlayerStatsComp()->GetHealAmount() > 0) {
				UGameplayStatics::ApplyDamage(this, -2 * GetPlayerStatsComp()->GetHealAmount(), nullptr, this, nullptr);
			}
			else {
				UGameplayStatics::ApplyDamage(this, -2, nullptr, this, nullptr);
			}
			RegenTimer = 0;
		}
	}
	if (IsDashing == false) {
		clearDirectionTimer += DeltaTime;
		if (clearDirectionTimer > directionhold) {
			LastKnownInput = FVector2D::Zero();
		}

		DashCDTimer += DeltaTime;
		if (DashCDTimer >= GetPlayerStatsComp()->GetDashCooldown()) {
			CanDash = true;
		}
	}

	if (IsDashing == true) {
		DashlengthTimer += DeltaTime;
		if (!(DashlengthTimer >= GetPlayerStatsComp()->GetDashLength())) {
			FVector Direction = (Forward * LastKnownInput.Y) + (Right * LastKnownInput.X);
			FVector Offset = Direction.GetSafeNormal() * this->GetCharacterMovement()->MaxWalkSpeed * DeltaTime;
			this->AddActorWorldOffset(Offset, true);
		}
		else {
			EnableCollision();
			SetPlayerSpeed(GetPlayerStatsComp()->GetMoveSpeed());
			IsDashing = false;
			DashCDTimer = 0;
			DashlengthTimer = 0;
		}
	}
}

// Called to bind functionality to input
void AUC_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(Moves, ETriggerEvent::Triggered, this, &AUC_Player::Move);
		EnhancedInputComponent->BindAction(Dashes, ETriggerEvent::Started, this, &AUC_Player::Dash);
		EnhancedInputComponent->BindAction(Paused, ETriggerEvent::Started, this, &AUC_Player::PlayerPause);
	}
}

void AUC_Player::SetOwnedWeapons(TArray<FOwnedWeapon>& OwnedWeapons)
{
	OwnedWeapons.Empty();
	for (auto It = WeaponInventory.begin(); It != WeaponInventory.end(); ++It)
	{
		if (ABaseWeapon* W = Cast<ABaseWeapon>(It->Value->GetChildActor()))
		{
			FOwnedWeapon Owned;
			Owned.Data = It->Key;
			Owned.Level = W->GetWeaponUpgrade() + 1;
			Owned.MaxLevel = WeaponSubsystem->GetCategoryUpgradeSize(It->Key);
			OwnedWeapons.Add(Owned);
		}
	}
}

void AUC_Player::PlayerLost()
{
	if (IAudioManagerInterface* I = Cast<IAudioManagerInterface>(GetWorld()->GetGameInstance()))
	{
		if (I->GetLibrary()->BGM_GameplaySong) {
			I->StopSound(I->GetLibrary()->BGM_GameplaySong);
		}

		if (I->GetLibrary()->BGM_ResultsMenuSong) {
			I->PlayMusic(I->GetLibrary()->BGM_ResultsMenuSong);
		}
	}
	else
	{
		UE_LOG(LogPlayer, Error, TEXT("No Audio Library Instance has been found!"));
	}

	if (APC) {
		APC->bShowMouseCursor = true;
		if (PlayerHUD) {
			PlayerHUD->RemoveFromParent();
		}
	}
}

UPlayerStatsComponent* AUC_Player::GetPlayerStatsComp()
{
	return PlayerStatsComponent;
}

void AUC_Player::SetWeapon(EWeaponCategory Type)
{
	FWeaponCreateParams P;
	P.Attachment = GetCapsuleComponent();
	P.Player = this;
	P.Type = Type;

	WeaponSubsystem->SetWeapon(P, WeaponInventory);
}

void AUC_Player::Move(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	if (IsDashing == false) {
		LastKnownInput = Input;
		AddMovementInput(Forward, Input.Y);
		AddMovementInput(Right, Input.X);
		clearDirectionTimer = 0;
	}

}
void AUC_Player::Dash(const FInputActionValue& Value)
{
	if (CanDash == true && LastKnownInput != FVector2D::Zero()) {
		IsDashing = true;
		CanDash = false;
		DisableCollision();
		SetPlayerSpeed(GetPlayerStatsComp()->GetMoveSpeed() * 4);
	}

}

void AUC_Player::PlayerPause(const FInputActionValue& Value)
{
	if (isPaused)
	{
		ResumeGame();
	}
	else
	{
		PauseGame();
	}
}

void AUC_Player::SetPlayerSpeed(float value)
{
	this->GetCharacterMovement()->MaxWalkSpeed = value;
}

void AUC_Player::EnableCollision()
{
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
}

void AUC_Player::DisableCollision()
{
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
}

void AUC_Player::LevelUpPause()
{
	APlayerController* PController = Cast<APlayerController>(GetController());
	if (!PController)
	{
		UE_LOG(LogPlayer, Error, TEXT("Error: Player Controller Cast failed!!!"));
		return;
	}
	if (isPaused)
	{
		return;
	}
	PController->SetPause(true);
	isPaused = true;
}

void AUC_Player::LevelUpResume()
{
	APlayerController* PController = Cast<APlayerController>(GetController());
	if (!PController || !isPaused)
	{
		return;
	}

	PController->SetPause(false);
	isPaused = false;
}

void AUC_Player::PauseGame()
{
	APlayerController* PController = Cast<APlayerController>(GetController());
	if (!PController)
	{
		UE_LOG(LogPlayer, Error, TEXT("Error: Player Controller Cast failed!!!"));
		return;
	}
	if (isPaused)
	{
		return;
	}

	PController->SetPause(true);
	isPaused = true;

	if (PlayerHUD)
	{
		PlayerHUD->isPaused = true;
		PlayerHUD->SetVisibility(ESlateVisibility::Hidden);
	}

	if (PauseHUD && !PauseHUD->IsInViewport())
	{
		PauseHUD->AddToViewport();
	}

	// Game And UI: the mouse can click the buttons, and Esc/P still reaches IA_Pause to resume
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	PController->SetInputMode(InputMode);
	PController->bShowMouseCursor = true;
}

void AUC_Player::ResumeGame()
{
	APlayerController* PController = Cast<APlayerController>(GetController());
	if (!PController || !isPaused)
	{
		return;
	}

	PController->SetPause(false);
	isPaused = false;

	if (PlayerHUD)
	{
		PlayerHUD->isPaused = false;
		PlayerHUD->SetVisibility(ESlateVisibility::Visible);
	}

	if (PauseHUD && PauseHUD->IsInViewport())
	{
		PauseHUD->RemoveFromParent();
	}

	PController->SetInputMode(FInputModeGameOnly());
	PController->bShowMouseCursor = false;
}

void AUC_Player::UpdatePickUpRadius(float value)
{
	PickUpRadius->SetSphereRadius(value);
}

void AUC_Player::HandleEXPOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult)
{
	if (OtherActor) {
		AEXP_PickUp* EXP_PickUp = Cast<AEXP_PickUp>(OtherActor);
		AHealth_PickUp* Health_PickUp = Cast< AHealth_PickUp>(OtherActor);
		if (EXP_PickUp || Health_PickUp) {
			AttractedActors.Add(OtherActor);
		}
		else {
			return;
		}
	}
}

UCapsuleComponent* AUC_Player::GetCapsuleComponent()
{
	return FindComponentByClass<UCapsuleComponent>();
}

USphereComponent* AUC_Player::GetRadiusComponent()
{
	return PickUpRadius;
}