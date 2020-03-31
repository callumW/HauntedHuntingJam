// Fill out your copyright notice in the Description page of Project Settings.

#include "ForestGenerator.h"


TCHAR const* TREE_MESH_PATHS[] = {
    // TEXT("StaticMesh'/Game/FirstPerson/Meshes/Environment/tree_1.tree_1'"),  // Scale of this model is kinda wrong
    TEXT("StaticMesh'/Game/FirstPerson/Meshes/Environment/tree_2.tree_2'"),
    TEXT("StaticMesh'/Game/FirstPerson/Meshes/Environment/tree_3.tree_3'"),
    TEXT("StaticMesh'/Game/FirstPerson/Meshes/Environment/tree_4.tree_4'")
};

ForestGenerator::ForestGenerator()
{
}

ForestGenerator::~ForestGenerator()
{
}

std::vector<map_chunk_t> ForestGenerator::GenerateForest(FVector const& size, int32 seed, int32 num_trees,
    FVector const& origin, TArray<ATreeGenerationBlockingVolume*> const& blocking_volumes)
{

    auto bounds = blocking_volumes[0]->GetBounds().GetBox();

    UE_LOG(LogTemp, Display, TEXT("blocking volume bounding box: (%f,%f,%f) -> (%f,%f,%f)"),
        bounds.Max.X, bounds.Max.Y, bounds.Max.Z, bounds.Min.X, bounds.Min.Y, bounds.Min.Z);

    UE_LOG(LogTemp, Display, TEXT("forest origin point: (%f,%f,%f)"), origin.X, origin.Y, origin.Z);

    /*
        Validate Input:
         * Verify blocking volumes to not encompass all of size.
         * verify num_tree is > 1
         * Verify size is positive
    */
    std::vector<map_chunk_t> map;

    float const width = size.X;
    float const height = size.Y;
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

    std::mt19937 random_engine(seed);

    auto get_x = std::bind(x_dist, std::mt19937(random_engine()));
    auto get_y = std::bind(y_dist, std::mt19937(random_engine()));
    auto get_rotation = std::bind(rot_dist, std::mt19937(random_engine()));
    auto get_mesh_path_idx = std::bind(mesh_dist, std::mt19937(random_engine()));

    size_t const num_points = static_cast<size_t>(num_trees);

    for (size_t i = 0; i < num_points; i++) {

        FVector tree_location;
        FVector tree_world_location;
        tree_world_location.Z = origin.Z;
        bool location_is_valid = true;
        do {
            location_is_valid = true;

            tree_location.X = get_x();
            tree_location.Y = get_y();

            tree_world_location.X = tree_location.X + origin.X;
            tree_world_location.Y = tree_location.Y + origin.Y;

            for (auto const& volume : blocking_volumes) {
                if (volume && volume->EncompassesPoint(tree_world_location)) {
                    location_is_valid = false;
                    break;
                }
            }
        }
        while (!location_is_valid);

        unsigned const mesh_idx = get_mesh_path_idx();

        check(mesh_idx < num_meshes && mesh_idx >= 0);

        tree_t tree = {tree_location, FRotator(0.0, get_rotation(), 0.0f),
            TREE_MESH_PATHS[get_mesh_path_idx()]};

        chunk.trees.push_back(tree);
    }

    map.push_back(chunk);

    return map;
}
