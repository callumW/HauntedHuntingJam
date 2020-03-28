// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include <random>

#include "CoreMinimal.h"
#include "Layout/SlateRect.h"	// For FSlateRect

typedef struct tree {
	FVector location;
	FRotator rotation;
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

	std::vector<map_chunk_t> GenerateForest();

private:
	std::mt19937 m_random_engine;
};
