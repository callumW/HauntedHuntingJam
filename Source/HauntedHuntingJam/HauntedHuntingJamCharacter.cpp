// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HauntedHuntingJamCharacter.h"
#include "HauntedHuntingJamProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "Engine/EngineTypes.h"
#include "TreeComponent.h"
#include "ForestBuilder.h"
#include "DrawDebugHelpers.h"
#include "HauntedHuntingJamHUD.h"
#include "FeedableFire.h"
#include "Readable.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AHauntedHuntingJamCharacter

WEAPON_MODE& operator++(WEAPON_MODE& cur_mode, int i)
{
	switch (cur_mode) {
		case WEAPON_MODE::GUN:
			return cur_mode = WEAPON_MODE::HANDS;
		case WEAPON_MODE::HANDS:
			return cur_mode = WEAPON_MODE::FLASHLIGHT;
		case WEAPON_MODE::FLASHLIGHT:
			return cur_mode = WEAPON_MODE::GUN;
		default:
			return cur_mode = WEAPON_MODE::GUN;
	}
}

AHauntedHuntingJamCharacter::AHauntedHuntingJamCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void AHauntedHuntingJamCharacter::BeginPlay()
{
	// Call the base class
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}

	GetFlashlight();

	UpdateWeaponMode();
}

void AHauntedHuntingJamCharacter::EscapeKeyPressed()
{
	UE_LOG(LogTemp, Display, TEXT("Escape key pressed!"));
	ClearHUD();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AHauntedHuntingJamCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Escape", IE_Released, this, &AHauntedHuntingJamCharacter::EscapeKeyPressed);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AHauntedHuntingJamCharacter::OnFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AHauntedHuntingJamCharacter::StopFire);

	// Bind weapon switch event
	PlayerInputComponent->BindAction("SwitchWeapon", IE_Pressed, this, &AHauntedHuntingJamCharacter::SwitchWeapon);

	// Toggle Flashlight
	PlayerInputComponent->BindAction("ToggleFlashlight", IE_Pressed, this, &AHauntedHuntingJamCharacter::ToggleFlashlight);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AHauntedHuntingJamCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AHauntedHuntingJamCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHauntedHuntingJamCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AHauntedHuntingJamCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AHauntedHuntingJamCharacter::LookUpAtRate);
}

/**
 * Called via input to switch the weapon
 */
void AHauntedHuntingJamCharacter::SwitchWeapon()
{
	if (is_firing) {	// Can't switch weapon whilst firing
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("Switch Weapon!"));
	weapon_mode++;
	time_since_last_fire = 9999.0f;		//reset last fire time;

	switch (weapon_mode) {
		case WEAPON_MODE::GUN:
			UE_LOG(LogTemp, Display, TEXT("Gun"));
			break;
		case WEAPON_MODE::HANDS:
			UE_LOG(LogTemp, Display, TEXT("Hands"));
			break;
		case WEAPON_MODE::FLASHLIGHT:
			UE_LOG(LogTemp, Display, TEXT("Flashlight"));
			break;
		default:
			UE_LOG(LogTemp, Display, TEXT("Unknown mode"));
	}

	UpdateWeaponMode();
}

void AHauntedHuntingJamCharacter::UpdateWeaponMode()
{
	switch (weapon_mode) {
		case WEAPON_MODE::GUN:
		{
			if (flashlight_mesh && flashlight) {
				flashlight_mesh->SetVisibility(false);
				flashlight->SetVisibility(false);
			}
			Mesh1P->SetVisibility(true);
			FP_Gun->SetVisibility(true);
			if (flashlight_off_sound && flashlight->IsVisible()) {
				UGameplayStatics::PlaySoundAtLocation(this, flashlight_off_sound,
					GetActorLocation());
			}
			break;
		}
		case WEAPON_MODE::HANDS:
		{
			Mesh1P->SetVisibility(false);
			FP_Gun->SetVisibility(false);
			break;
		}
		case WEAPON_MODE::FLASHLIGHT:
		{
			Mesh1P->SetVisibility(false);
			FP_Gun->SetVisibility(false);
			if (flashlight_mesh && flashlight) {
				flashlight_mesh->SetVisibility(true);
				flashlight->SetVisibility(true);
			}
			if (flashlight_on_sound) {
				UGameplayStatics::PlaySoundAtLocation(this, flashlight_on_sound,
					GetActorLocation());
			}
			break;
		}
	}
}

void AHauntedHuntingJamCharacter::ShootGun()
{
	if (time_since_last_fire < gun_fire_rate) {
		return;
	}
	time_since_last_fire = 0;

	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<AHauntedHuntingJamProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<AHauntedHuntingJamProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AHauntedHuntingJamCharacter::FindUsableObject()
{
	auto world = GetWorld();

	FHitResult hit_result{EForceInit::ForceInit};
	FCollisionQueryParams query_params{TEXT("use_raytrace"), true, this};

	/*
		Right now we get the characters eye level and extend in the direction of the camera to get
		the end location of our raycast. This isn't quite dead-on for the cursor but it will do for
		now.
	*/

	FVector const start = FirstPersonCameraComponent->GetComponentLocation();
	FRotator const rotation = GetControlRotation();

	FVector const dir = rotation.RotateVector(FVector::ForwardVector);
	FVector end = start + dir * raycast_distance;

	if (show_raycast_debug) {
		DrawDebugLine(world, start, end, FColor::Red, false, 2.0f);
	}


	if (world->LineTraceSingleByChannel(hit_result, start, end, ECollisionChannel::ECC_WorldStatic,
		query_params, FCollisionResponseParams::DefaultResponseParam)) {

		auto actor = hit_result.GetActor();
		auto hit_component = hit_result.GetComponent();

		if (hit_component && actor) {

			if (actor->GetClass() == AForestBuilder::StaticClass()) {
				AForestBuilder* forest = dynamic_cast<AForestBuilder*>(actor);
				UTreeComponent* tree = dynamic_cast<UTreeComponent*>(hit_component);

				if (tree && forest && weapon_mode == WEAPON_MODE::HANDS) {
					AttackTree(forest, tree);
				}
			}
			else if (actor->GetClass() == AFeedableFire::StaticClass()) {
				AFeedableFire* fire = dynamic_cast<AFeedableFire*>(actor);
				if (fire) {
					wood_count -= fire->Feed(wood_count);
					UpdateHUD();
				}
			}
			else if (actor->GetClass() == AReadable::StaticClass()) {
				AReadable* sign = dynamic_cast<AReadable*>(actor);
				if (sign) {
					sign->Read();
				}
			}
			else {
				UE_LOG(LogTemp, Display, TEXT("Actor is unknown type"));
			}
		}
	}
}

void AHauntedHuntingJamCharacter::AttackTree(AForestBuilder* forest, UTreeComponent* tree)
{
	if (time_since_last_fire < wood_harvest_rate) {
		return;
	}
	time_since_last_fire = 0;

	if (forest->HitTree(tree)) {
		wood_count++;
		UpdateHUD();
	}
}

void AHauntedHuntingJamCharacter::UpdateHUD()
{
	if (Controller) {
		APlayerController* player_controller =
			dynamic_cast<APlayerController*>(Controller);
		if (player_controller) {
			// Update HUD
			AHauntedHuntingJamHUD* HUD =
				dynamic_cast<AHauntedHuntingJamHUD*>(player_controller->GetHUD());

			if (HUD) {
				HUD->UpdateWoodCount(wood_count);
			}
		}
	}
}


void AHauntedHuntingJamCharacter::Use()
{
	FindUsableObject();
}

void AHauntedHuntingJamCharacter::OnFire()
{
	is_firing = true;
	FireLogic();
}

void AHauntedHuntingJamCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AHauntedHuntingJamCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AHauntedHuntingJamCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AHauntedHuntingJamCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void AHauntedHuntingJamCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AHauntedHuntingJamCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AHauntedHuntingJamCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AHauntedHuntingJamCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AHauntedHuntingJamCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AHauntedHuntingJamCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AHauntedHuntingJamCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AHauntedHuntingJamCharacter::TouchUpdate);
		return true;
	}

	return false;
}

void AHauntedHuntingJamCharacter::StopFire()
{
	is_firing = false;
	UE_LOG(LogTemp, Display, TEXT("Stop firing"));
}

void AHauntedHuntingJamCharacter::Tick(float delta_seconds)
{
	time_since_last_fire += delta_seconds;
	if (is_firing) {
		FireLogic();
	}
}

void AHauntedHuntingJamCharacter::FireLogic()
{
	switch (weapon_mode) {
		case WEAPON_MODE::GUN:
			ShootGun();
			break;
		case WEAPON_MODE::HANDS:
			Use();
			break;
		case WEAPON_MODE::FLASHLIGHT:
			Use();
			break;
		default:
			UE_LOG(LogTemp, Display, TEXT("Fire() when in unknown WEAPON_MODE"));
	}
}

void AHauntedHuntingJamCharacter::GetFlashlight()
{
	TArray<USceneComponent*> children;

	RootComponent->GetChildrenComponents(true, children);

	FString const flashlight_name = "flashlight";
	FString const flashlight_mesh_name = "flashlight_mesh";

	for (auto child : children) {
		if (child->GetFName().ToString() == flashlight_name) {
			UE_LOG(LogTemp, Display, TEXT("Found flashlight"));

			USpotLightComponent* tmp = dynamic_cast<USpotLightComponent*>(child);

			if (tmp) {
				flashlight = tmp;
			}
		}
		else if (child->GetFName().ToString() == flashlight_mesh_name) {

			UStaticMeshComponent* tmp = dynamic_cast<UStaticMeshComponent*>(child);

			if (tmp) {
				flashlight_mesh = tmp;
			}
		}
	}
}

void AHauntedHuntingJamCharacter::ToggleFlashlight()
{
	if (flashlight && flashlight_mesh && weapon_mode == WEAPON_MODE::FLASHLIGHT) {
		flashlight->ToggleVisibility();
		if (flashlight->IsVisible()) {
			if (flashlight_on_sound) {
				UGameplayStatics::PlaySoundAtLocation(this, flashlight_on_sound,
					GetActorLocation());
			}
		}
		else {
			if (flashlight_off_sound) {
				UGameplayStatics::PlaySoundAtLocation(this, flashlight_off_sound,
					GetActorLocation());
			}
		}
	}
}

void AHauntedHuntingJamCharacter::ClearHUD()
{
	if (Controller) {
		APlayerController* player_controller =
			dynamic_cast<APlayerController*>(Controller);
		if (player_controller) {
			// Update HUD
			AHauntedHuntingJamHUD* HUD =
				dynamic_cast<AHauntedHuntingJamHUD*>(player_controller->GetHUD());

			if (HUD) {
				HUD->DisplayReadableTexture(nullptr);
			}
		}
	}
}
