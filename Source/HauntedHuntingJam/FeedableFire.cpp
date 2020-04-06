// Fill out your copyright notice in the Description page of Project Settings.


#include "FeedableFire.h"

// Sets default values
AFeedableFire::AFeedableFire()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (!RootComponent) RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("fire_root"));
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
