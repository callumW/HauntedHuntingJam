// Fill out your copyright notice in the Description page of Project Settings.

#include <array>

#include "Misc/FileHelper.h"
#include "Math/Color.h"

#include "ForestBuilder.h"

// Sets default values
AForestBuilder::AForestBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (!RootComponent) RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("forest_root"));
}

void AForestBuilder::Tick(float delta_seconds)
{
	static FVector last_update_location{0.0f};
	if (player) {
		auto const location = player->GetActorLocation();
		auto const dist_from_last_update = last_update_location - location;

		if (dist_from_last_update.Size() > movement_threshold) {
			UE_LOG(LogTemp, Display, TEXT("Culling Trees"));
			last_update_location = location;
			UpdateVisibleTrees(location);
		}
	}
}

void AForestBuilder::OnConstruction(FTransform const& transform)
{
	if (spawn_in_editor) {
		Build();
	}
}

void AForestBuilder::PostEditChangeProperty(FPropertyChangedEvent & prop_change_event)
{
	Refresh();
}

void AForestBuilder::Build()
{
	// TODO Validate parameters!
	auto map = m_generator.GenerateForest(size, seed, num_trees, GetActorLocation(), blocking_volumes);

	grid_ele_size_x = size.X / num_divisions_culling;
	grid_ele_size_y = size.Y / num_divisions_culling;

	auto const origin_location = GetActorLocation();

	offset = origin_location - size / 2.0f;


	for (float x = 0.0f; x < size.X; x += grid_ele_size_x) {
		for (float y = 0.0f; y < size.Y; y += grid_ele_size_y) {
			culling_grid.Add({{}, FVector{x+offset.X, y+offset.Y, origin_location.Z}});
		}
	}

	UE_LOG(LogTemp, Display, TEXT("culling grid size: %u"), culling_grid.Num());

	for (auto const& chunk : map) {
		SpawnTrees(chunk);
	}
}

void AForestBuilder::Refresh()
{
	DeleteAllTrees();

	if (spawn_in_editor) {
		Build();
	}
}

void AForestBuilder::DeleteAllTrees()
{
	culling_grid.Empty();
	while (trees.Num() > 0) {
		auto comp = trees.Pop();
		comp->DestroyComponent();
	}
}

void AForestBuilder::ClearVisibleTrees()
{
	for (auto & ele : visible_last_round) {
		ele->SetVisibility(false);
	}

	visible_last_round.Empty();
}

void AForestBuilder::UpdateVisibleTrees(FVector const& player_location)
{
	ClearVisibleTrees();
	TArray<culling_chunk_t*> chunks;

	GetCullingGridChunks(player_location, chunks);
	for (auto & chunk : chunks) {
		if (chunk) {
			for (auto & ele : chunk->elements) {
				UE_LOG(LogTemp, Display, TEXT("Setting tree as visible"));
				ele->SetVisibility(true);
				visible_last_round.Add(ele);
			}
		}
	}
}

// Called when the game starts or when spawned
void AForestBuilder::BeginPlay()
{
	Super::BeginPlay();
	if (!spawn_in_editor) {
		Build();
	}
}

void AForestBuilder::SpawnTrees(map_chunk_t const& chunk)
{
	for (auto & tree : chunk.trees) {
		auto obj = SpawnTreeAt(tree);
		if (obj) {
			auto grid_chunk = GetCullingGridChunk(obj->GetComponentLocation());
			if (grid_chunk) {
				obj->SetVisibility(false);
				grid_chunk->elements.Add(obj);
			}
			else {
				UE_LOG(LogTemp, Display, TEXT("Failed to get grid chunk"));
			}
		}
	}
}

FVector2D AForestBuilder::GetCullingGridCoord(FVector const& loc)
{
	auto adjust_loc = loc - offset;

	float x = adjust_loc.X / grid_ele_size_x;
	float y = adjust_loc.Y / grid_ele_size_y;

	return FVector2D{x, y};
}

culling_chunk_t* AForestBuilder::GetCullingGridChunk(FVector const& loc)
{
	auto coord = GetCullingGridCoord(loc);

	int32 x = static_cast<int32>(coord.X);
	int32 y = static_cast<int32>(coord.Y);

	UE_LOG(LogTemp, Display, TEXT("Calculated chunk grid: (%d, %d) for (%f,%f,%f)"), x, y, loc.X, loc.Y, loc.Z);

	if ((x < 0) || (y < 0) || (x * num_divisions_culling + y > culling_grid.Num())) {
		return nullptr;
	}

	return culling_grid.GetData() + x * num_divisions_culling + y;
}

void AForestBuilder::GetCullingGridChunks(FVector const& loc, TArray<culling_chunk_t*>& chunks)
{
	int32 const render_dist_x = static_cast<int32>(render_distance / grid_ele_size_x);
	int32 const render_dist_y = static_cast<int32>(render_distance / grid_ele_size_y);

	auto coords = GetCullingGridCoord(loc);

	int32 x = static_cast<int32>(coords.X);
	int32 y = static_cast<int32>(coords.Y);

	int32 min_x = x - render_dist_x;
	int32 max_x = x + render_dist_x;
	int32 min_y = y - render_dist_y;
	int32 max_y= y + render_dist_y;

	if (min_x < 0) min_x = 0;
	if (max_x >= num_divisions_culling) max_x = num_divisions_culling - 1;

	if (min_y < 0) min_y = 0;
	if (max_y >= num_divisions_culling) max_y = num_divisions_culling - 1;

	for (int32 x_idx = min_x; x_idx <= max_x; x_idx++) {
		for (int32 y_idx = min_y; y_idx <= max_y; y_idx++) {
			chunks.Add(culling_grid.GetData() + x_idx * num_divisions_culling + y_idx);
		}
	}
}

USceneComponent* AForestBuilder::SpawnTreeAt(tree_t const& tree)
{
	if (RootComponent) {

		FVector const& loc = tree.location;
		FRotator const& rot = tree.rotation;
		TCHAR const* mesh_path = tree.mesh_path;

		check(mesh_path != nullptr);

		FString object_name = TEXT("tree");
		object_name.Appendf(TEXT("@(%f,%f,%f)"), loc.X, loc.Y, loc.Z);
		auto tree_obj = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(),
			*object_name);

		if (tree_obj) {

			UStaticMesh* mesh = (UStaticMesh*) StaticLoadObject(UStaticMesh::StaticClass(), nullptr,
				mesh_path);

			if (mesh) {
				tree_obj->SetStaticMesh(mesh);

				// UE_LOG(LogTemp, Display, TEXT("SpawN TREE! @ (%f,%f,%f)"), loc.X, loc.Y, loc.Z);
				trees.Add(tree_obj);
				tree_obj->RegisterComponent();
				tree_obj->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

				tree_obj->SetRelativeLocation(loc, false);
				tree_obj->AddLocalRotation(rot, false);

				return tree_obj;
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Failed to create mesh!"));
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Failed to create tree!"));
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("No Room component!"));
	}

	return nullptr;
}

void AForestBuilder::OutputToBMP(std::vector<map_chunk_t> const& map, FString filename_base)
{
	for (auto & chunk : map) {
		FString filename = filename_base;
		filename.Appendf(TEXT("%.0f_%.0f.bmp"), chunk.rect.Left, chunk.rect.Bottom);
		OutputToBMP(chunk, filename);
	}
}

void AForestBuilder::OutputToBMP(map_chunk_t const& map, FString filename)
{
	auto const map_size = map.rect.GetSize();

	int32 width = static_cast<int32>(map_size.X);
	int32 height = static_cast<int32>(map_size.Y);

	UE_LOG(LogTemp, Display, TEXT("Outputting map chunk: (%f, %f, %f, %f) -> %dx%d"),
		map.rect.Left,
		map.rect.Top,
		map.rect.Right,
		map.rect.Bottom,
		width,
		height);

	size_t const num_pixels = static_cast<size_t>(width * height);

	std::vector<struct FColor> pixel_data = {num_pixels, FColor(0)};

	for (auto & tree : map.trees) {
		float x_raw = tree.location.X + width / 2.0f;
		float y_raw = tree.location.Y + height / 2.0f;

		check(x_raw > 0.0f && y_raw > 0.0f);
		size_t x = static_cast<size_t>(x_raw);
		size_t y = static_cast<size_t>(y_raw);

		size_t const idx = y * width + x;
		if (idx < num_pixels) {
			pixel_data[idx].R = 255;	// set to red
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Skipping tree point as it is out of bounds!"));
		}
	}

	IFileManager& file_manager = IFileManager::Get();
	if (!FFileHelper::CreateBitmap(*filename, width, height, pixel_data.data(),
		nullptr, &file_manager, nullptr, false)) {
		UE_LOG(LogTemp, Display, TEXT("Failed to create bitmap"));
	}
}
