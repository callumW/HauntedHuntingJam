// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ForestGenerator.h"
#include "TreeGenerationBlockingVolume.h"


#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ForestBuilder.generated.h"


typedef struct culling_chunk {
	TArray<USceneComponent*> elements;
	FVector location;
} culling_chunk_t;

UCLASS()
class HAUNTEDHUNTINGJAM_API AForestBuilder : public AActor
{
	GENERATED_BODY()

	ForestGenerator m_generator;

	void OutputToBMP(std::vector<map_chunk_t> const& map, FString filename_base);
	void OutputToBMP(map_chunk_t const& map, FString filename);

	void SpawnTrees(map_chunk_t const& chunk);

	USceneComponent* SpawnTreeAt(tree_t const& tree);

	void UpdateVisibleTrees(FVector const& player_location);

	void Build();

	void Refresh();

	void DeleteAllTrees();

	culling_chunk_t* GetCullingGridChunk(FVector const& loc);

	TArray<USceneComponent*> trees;

	TArray<culling_chunk_t> culling_grid;
	float grid_ele_size_x = 0.0f;
	float grid_ele_size_y = 0.0f;
	FVector offset;

	TArray<USceneComponent*> visible_last_round;

	UPROPERTY(EditAnywhere)
	AActor* player;

	UPROPERTY(EditAnywhere, Category = "Gameplay", meta=(ToolTip="Maximum distance from player before hiding trees"))
	float render_distance = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Gameplay", meta=(ToolTip="Maximum distance player can move before tree culling takes place"))
	float movement_threshold = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Gameplay", meta=(ToolTip="Number of grid divisions along each axis for culling grid"))
	int32 num_divisions_culling = 12;

	UPROPERTY(EditAnywhere, Category = "Gameplay")
	bool spawn_in_editor = false;

	UPROPERTY(EditAnywhere, Category = "Generation")
	FVector size = FVector(1000.0f, 1000.0f, 1000.0f);

	UPROPERTY(EditAnywhere, Category = "Generation")
	int32 seed = 3453;

	UPROPERTY(EditAnywhere, Category = "Generation")
	int32 num_trees = 300;

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
