// Fill out your copyright notice in the Description page of Project Settings.


#include "FeedableFire.h"

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
}

// Called every frame
void AFeedableFire::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
