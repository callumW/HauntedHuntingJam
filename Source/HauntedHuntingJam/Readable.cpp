// Fill out your copyright notice in the Description page of Project Settings.


#include "Readable.h"

// Sets default values
AReadable::AReadable()
{
	PrimaryActorTick.bCanEverTick = false;

	if (!RootComponent) RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("root"));

	if (mesh) {
		mesh_component = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("mesh_component"));

		mesh_component->SetStaticMesh(mesh);

		mesh_component->SetupAttachment(RootComponent);
	}
}

void AReadable::PostEditChangeProperty(FPropertyChangedEvent & prop_change_event)
{
	// Reset mesh component
	if (mesh_component) {
		mesh_component->DestroyComponent();
		mesh_component = nullptr;
	}

	if (mesh) {
		mesh_component =
			NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass());

		mesh_component->SetStaticMesh(mesh);
		mesh_component->RegisterComponent();
		mesh_component->AttachToComponent(RootComponent,
			FAttachmentTransformRules::KeepRelativeTransform);

		mesh_component->SetRelativeLocation(FVector(0.0f), false);
	}
}


// Called when the game starts or when spawned
void AReadable::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AReadable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AReadable::Read()
{
	UE_LOG(LogTemp, Display, TEXT("Read!"));
}
