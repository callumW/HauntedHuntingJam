// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FeedableFire.generated.h"

UCLASS()
class HAUNTEDHUNTINGJAM_API AFeedableFire : public AActor
{
private:
	GENERATED_BODY()

	FVector starting_fire_scale;

	UAudioComponent* audio_comp;

	void UpdateHUD();

	void PlayFeedSound();

public:
	UPROPERTY(EditAnywhere)
	UParticleSystemComponent* fire_particle_system = nullptr;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* container_mesh = nullptr;

	UPROPERTY(EditAnywhere)
	USoundBase* use_sound = nullptr;

	UPROPERTY(EditAnywhere, Category="Gameplay", meta=(ToolTip="Actor that represents the fires area of effect"))
	AActor* area_of_effect_actor = nullptr;

	FVector starting_area_effect_scale;

	UPROPERTY(EditAnywhere, Category="Gameplay", meta=(ToolTip="Fuel usage in fuel/sec"))
	float fuel_burn_rate = 1.0f;

	UPROPERTY(EditAnywhere)
	float fuel_count = 20;

	UPROPERTY(EditAnywhere)
	float max_fuel_count = 20;

	// Sets default values for this actor's properties
	AFeedableFire();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// return true if feed was successful (e.g. fire is still alive)
	int32 Feed(int32 wood_count);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
