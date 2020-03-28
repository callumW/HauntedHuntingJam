// Fill out your copyright notice in the Description page of Project Settings.


#include "ForestGenerator.h"

ForestGenerator::ForestGenerator()
{
}

ForestGenerator::~ForestGenerator()
{
}

std::vector<map_chunk_t> ForestGenerator::GenerateForest()
{
    std::vector<map_chunk_t> map;

    float const width = 4000.0f;
    float const height = 4000.0f;

    map_chunk_t chunk;
    chunk.rect = FSlateRect(0 - (width / 2.0f), (height / 2.0f), (width / 2.0f), 0 - (height / 2.0f));

    std::uniform_real_distribution<float> x_dist(chunk.rect.Left, chunk.rect.Right);
    std::uniform_real_distribution<float> y_dist(chunk.rect.Bottom, chunk.rect.Top);

    auto get_x = std::bind(x_dist, m_random_engine);
    auto get_y = std::bind(y_dist, m_random_engine);

    size_t const num_points = 30;

    for (size_t i = 0; i < num_points; i++) {
        tree_t tree = {FVector(get_x(), get_y(), 0.0f), FRotator(0.0f)};
        chunk.trees.push_back(tree);
    }

    return map;
}
