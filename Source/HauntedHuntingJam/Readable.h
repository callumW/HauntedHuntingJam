// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Templates/SharedPointer.h"
#include "Engine/StaticMesh.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Readable.generated.h"

UCLASS()
class HAUNTEDHUNTINGJAM_API AReadable : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Appearance")
	UStaticMesh* mesh = nullptr;


	UPROPERTY()
	UStaticMeshComponent* mesh_component = nullptr;
public:
	// Sets default values for this actor's properties
	AReadable();

	virtual void Tick(float DeltaTime) override;

	void Read();


	virtual void PostEditChangeProperty(FPropertyChangedEvent & prop_change_event);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
