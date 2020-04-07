// Fill out your copyright notice in the Description page of Project Settings.


#include "FeedableFire.h"

#include "HauntedHuntingJamHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"

// Sets default values
AFeedableFire::AFeedableFire()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (!RootComponent) RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("fire_root"));

	if (container_mesh) container_mesh->SetupAttachment(RootComponent);

	if (fire_particle_system) fire_particle_system->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AFeedableFire::BeginPlay()
{
	Super::BeginPlay();

	if (fire_particle_system) {
		starting_fire_scale = fire_particle_system->GetRelativeScale3D();
	}

	auto children = RootComponent->GetAttachChildren();

	for (auto child : children) {
		audio_comp = dynamic_cast<UAudioComponent*>(child);
		if (audio_comp) {
			break;
		}
	}
}

// Called every frame
void AFeedableFire::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	fuel_count -= fuel_burn_rate * DeltaTime;

	if (fuel_count < 0.0f) {
		fuel_count = 0.0f;
	}

	UpdateHUD();

	float new_scale_scalar = fuel_count / max_fuel_count;
	// Shrink fire
	if (fire_particle_system) {
		FVector new_scale = starting_fire_scale * new_scale_scalar;

		UE_LOG(LogTemp, Display, TEXT("scaling fire to: %f,%f,%f"), new_scale.X, new_scale.Y,
			new_scale.Z);

		fire_particle_system->SetRelativeScale3D(new_scale);
	}

	if (audio_comp) {
		UE_LOG(LogTemp, Display, TEXT("New audio volume scalar: %f"), new_scale_scalar);

		audio_comp->SetVolumeMultiplier(new_scale_scalar);
	}
}

void AFeedableFire::UpdateHUD()
{
	auto world = GetWorld();

	if (world) {
		auto player_controller = world->GetFirstPlayerController();
		if (player_controller) {
			AHauntedHuntingJamHUD* HUD =
				dynamic_cast<AHauntedHuntingJamHUD*>(player_controller->GetHUD());

			HUD->UpdateFireLevel(fuel_count / max_fuel_count);
		}
	}
}

int32 AFeedableFire::Feed(int32 wood_count)
{
	UE_LOG(LogTemp, Display, TEXT("Feeding fire: cur_fuel: %f, max_fuel: %f, incoming_fuel: %d"),
		fuel_count, max_fuel_count, wood_count);
	float incoming_fuel = static_cast<float>(wood_count);

	float empty_fuel = max_fuel_count - fuel_count;
	if (empty_fuel >= 1.0f) { 	//Verify we can add a whole amount of fuel
		// Calculate used amount of fuel
		empty_fuel = FMath::RoundToZero(empty_fuel);
		if (incoming_fuel > empty_fuel) {
			incoming_fuel = empty_fuel;
		}

		fuel_count += incoming_fuel;

		return static_cast<int32>(incoming_fuel);
	}
	return 0;
}
