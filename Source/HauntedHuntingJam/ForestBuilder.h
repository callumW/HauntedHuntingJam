// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ForestGenerator.h"

#include "Containers/Array.h"

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

	void SpawnTrees(map_chunk_t const& chunk);

	void SpawnTreeAt(tree_t const& tree);

	void UpdateVisibleTrees(FVector const& player_location);

	TArray<USceneComponent*> trees;

	UPROPERTY(EditAnywhere)
	AActor* player;

	UPROPERTY(EditAnywhere)
	float render_distance = 1000.0f;

public:
	// Sets default values for this actor's properties
	AForestBuilder();

	virtual void Tick(float delta_seconds) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
