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
    // Note: this is a UI Rect so origin is top left
    chunk.rect = FSlateRect(0 - (width / 2.0f), 0 - (height / 2.0f), (width / 2.0f), (height / 2.0f));

    std::uniform_real_distribution<float> x_dist(chunk.rect.Left, chunk.rect.Right);
    std::uniform_real_distribution<float> y_dist(chunk.rect.Top, chunk.rect.Bottom);
    std::uniform_real_distribution<float> rot_dist(0.0f, 360.0f);

    std::mt19937 x_engine(m_random_engine());
    std::mt19937 y_engine(m_random_engine());
    std::mt19937 rot_engine(m_random_engine());

    auto get_x = std::bind(x_dist, x_engine);
    auto get_y = std::bind(y_dist, m_random_engine);
    auto get_rotation = std::bind(rot_dist, rot_engine);

    size_t const num_points = 100;

    for (size_t i = 0; i < num_points; i++) {
        tree_t tree = {FVector(get_x(), get_y(), 0.0f), FRotator(0.0, get_rotation(), 0.0f)};
        chunk.trees.push_back(tree);
    }

    map.push_back(chunk);

    return map;
}
