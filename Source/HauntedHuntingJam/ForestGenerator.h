// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TreeGenerationBlockingVolume.h"

#include <vector>
#include <random>

#include "CoreMinimal.h"
#include "Layout/SlateRect.h"	// For FSlateRect
#include "Containers/Array.h"


extern TCHAR const* TREE_MESH_PATHS[];

typedef struct tree {
	FVector location;
	FRotator rotation;
	TCHAR const* mesh_path = nullptr;
} tree_t;

typedef struct map_chunk {
	std::vector<tree_t> trees;
	FSlateRect rect;
} map_chunk_t;

/**
 *	Generate forest data
 */
class HAUNTEDHUNTINGJAM_API ForestGenerator
{
public:
	ForestGenerator();
	~ForestGenerator();

	std::vector<map_chunk_t> GenerateForest(FVector const& size, int32 seed, int32 num_trees,
		FVector const& origin, TArray<ATreeGenerationBlockingVolume*> const& blocking_volumes);
};
