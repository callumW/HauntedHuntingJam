// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ForestGenerator.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ForestBuilder.generated.h"

UCLASS()
class HAUNTEDHUNTINGJAM_API AForestBuilder : public AActor
{
	GENERATED_BODY()

	ForestGenerator m_generator;

	void OutputToBMP(std::vector<map_chunk_t> const& map, FString filename_base);
	void OutputToBMP(map_chunk_t const& map, FString filename);

public:
	// Sets default values for this actor's properties
	AForestBuilder();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
