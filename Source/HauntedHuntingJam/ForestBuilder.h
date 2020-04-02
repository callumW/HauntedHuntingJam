// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ForestGenerator.h"
#include "TreeGenerationBlockingVolume.h"

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

	void Build();

	void Refresh();

	void DeleteAllTrees();

	TArray<USceneComponent*> trees;

	UPROPERTY(EditAnywhere)
	AActor* player;

	UPROPERTY(EditAnywhere, Category = "Gameplay", meta=(ToolTip="Maximum distance from player before hiding trees"))
	float render_distance = 1000.0f;

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
