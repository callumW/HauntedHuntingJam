// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ForestGenerator.h"
#include "TreeGenerationBlockingVolume.h"

#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ForestBuilder.generated.h"

static float const DEFAULT_TREE_RENDER_DIST = 5000.0f;
static int32 const DEFAULT_NUM_TREES = 300;
static float const DEFAULT_SPAWN_AREA_DIM = 10000.0f;

UCLASS()
class HAUNTEDHUNTINGJAM_API AForestBuilder : public AActor
{
	GENERATED_BODY()

	ForestGenerator m_generator;

	void OutputToBMP(std::vector<map_chunk_t> const& map, FString filename_base);
	void OutputToBMP(map_chunk_t const& map, FString filename);

	void SpawnTrees(map_chunk_t const& chunk);

	void SpawnTreeAt(tree_t const& tree);

	void Build();

	void Refresh();

	void ValidateParameters();

	void DeleteAllTrees();

	TArray<USceneComponent*> trees;

	UPROPERTY(EditAnywhere)
	AActor* player;

	UPROPERTY(EditAnywhere, Category = "Gameplay", meta=(ToolTip="Maximum distance from player before hiding trees"))
	float render_distance = DEFAULT_TREE_RENDER_DIST;

	UPROPERTY(EditAnywhere, Category = "Gameplay")
	bool spawn_in_editor = false;

	UPROPERTY(EditAnywhere, Category = "Generation")
	FVector size = FVector(DEFAULT_SPAWN_AREA_DIM, DEFAULT_SPAWN_AREA_DIM, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Generation")
	int32 seed = 3453;

	UPROPERTY(EditAnywhere, Category = "Generation")
	int32 num_trees = DEFAULT_NUM_TREES;

	UPROPERTY(EditAnywhere, Category = "Generation")
	TArray<ATreeGenerationBlockingVolume*> blocking_volumes;

public:
	// Sets default values for this actor's properties
	AForestBuilder();

	virtual void Tick(float delta_seconds) override;

	virtual void OnConstruction(FTransform const& transform);

	virtual void PostEditChangeProperty(FPropertyChangedEvent & prop_change_event);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
