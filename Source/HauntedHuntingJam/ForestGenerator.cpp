// Fill out your copyright notice in the Description page of Project Settings.

#include "ForestGenerator.h"

TCHAR const* TREE_MESH_PATHS[] = {
	TEXT("StaticMesh'/Game/FirstPerson/Meshes/Environment/tree_3.tree_3'"),
	TEXT("StaticMesh'/Game/FirstPerson/Meshes/Environment/tree_4.tree_4'")
};

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
    size_t const num_meshes = sizeof(TREE_MESH_PATHS) / sizeof(TCHAR*);

    map_chunk_t chunk;
    // Note: this is a UI Rect so origin is top left
    chunk.rect = FSlateRect(0 - (width / 2.0f), 0 - (height / 2.0f), (width / 2.0f), (height / 2.0f));

    std::uniform_real_distribution<float> x_dist(chunk.rect.Left, chunk.rect.Right);
    std::uniform_real_distribution<float> y_dist(chunk.rect.Top, chunk.rect.Bottom);
    std::uniform_real_distribution<float> rot_dist(0.0f, 360.0f);
    std::uniform_int_distribution<unsigned> mesh_dist(0, num_meshes - 1);

    UE_LOG(LogTemp, Display, TEXT("Number of mesh paths: %u"), num_meshes);

    check(num_meshes > 1);

    auto get_x = std::bind(x_dist, std::mt19937(m_random_engine()));
    auto get_y = std::bind(y_dist, std::mt19937(m_random_engine()));
    auto get_rotation = std::bind(rot_dist, std::mt19937(m_random_engine()));
    auto get_mesh_path_idx = std::bind(mesh_dist, std::mt19937(m_random_engine()));

    size_t const num_points = 100;

    for (size_t i = 0; i < num_points; i++) {
        unsigned const mesh_idx = get_mesh_path_idx();

        check(mesh_idx < num_meshes && mesh_idx >= 0);

        tree_t tree = {FVector(get_x(), get_y(), 0.0f), FRotator(0.0, get_rotation(), 0.0f),
            TREE_MESH_PATHS[get_mesh_path_idx()]};

        chunk.trees.push_back(tree);
    }

    map.push_back(chunk);

    return map;
}
