// Fill out your copyright notice in the Description page of Project Settings.

#include <array>

#include "Misc/FileHelper.h"
#include "Math/Color.h"

#include "ForestBuilder.h"

// Sets default values
AForestBuilder::AForestBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	if (!RootComponent) RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("forest_root"));
}

void AForestBuilder::Tick(float delta_seconds)
{

}

void AForestBuilder::OnConstruction(FTransform const& transform)
{
	if (spawn_in_editor) {
		Build();
	}
}

void AForestBuilder::PostEditChangeProperty(FPropertyChangedEvent & prop_change_event)
{
	ValidateParameters();
	Refresh();
}

void AForestBuilder::ValidateParameters()
{
	if (render_distance < 0.0f) {
		UE_LOG(LogTemp, Warning, TEXT("Cannot have negative render distance: %f"), render_distance);
		render_distance = DEFAULT_TREE_RENDER_DIST;
	}

	if (num_trees < 0) {
		UE_LOG(LogTemp, Warning, TEXT("Cannot have negative number of trees: %f"), num_trees);
		num_trees = DEFAULT_NUM_TREES;
	}

	if (size.X < 0.0f) {
		UE_LOG(LogTemp, Warning, TEXT("Cannot have negative width of spawn area: %f"), size.X);
		size.X = DEFAULT_SPAWN_AREA_DIM;
	}

	if (size.Y < 0.0f) {
		UE_LOG(LogTemp, Warning, TEXT("Cannot have negative height of spawn area: %f"), size.Y);
		size.Y = DEFAULT_SPAWN_AREA_DIM;
	}
}

void AForestBuilder::Build()
{
	ValidateParameters();
	auto map = m_generator.GenerateForest(size, seed, num_trees, GetActorLocation(), blocking_volumes);

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
	while (trees.Num() > 0) {
		auto comp = trees.Pop();
		comp->DestroyComponent();
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
		SpawnTreeAt(tree);
	}
}

void AForestBuilder::SpawnTreeAt(tree_t const& tree)
{
	if (RootComponent) {

		FVector const& loc = tree.location;
		FRotator const& rot = tree.rotation;
		TCHAR const* mesh_path = tree.mesh_path;

		check(mesh_path != nullptr);

		auto tree_obj = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass());

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

				tree_obj->LDMaxDrawDistance = render_distance;
				tree_obj->SetCachedMaxDrawDistance(render_distance);
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
