// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FeedableFire.generated.h"

UCLASS()
class HAUNTEDHUNTINGJAM_API AFeedableFire : public AActor
{
private:
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	class UParticleSystemComponent* fire_particle_system;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* container_mesh;

public:
	// Sets default values for this actor's properties
	AFeedableFire();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
