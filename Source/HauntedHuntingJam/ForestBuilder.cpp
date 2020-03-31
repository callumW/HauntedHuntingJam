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
	// if (player) {
	// 	auto const location = player->GetActorLocation();
	//
	// 	// UE_LOG(LogTemp, Display, TEXT("Player is @ (%f,%f,%f)"), location.X, location.Y, location.Z);
	// 	UpdateVisibleTrees(location);
	// }
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
	auto map = m_generator.GenerateForest(size, seed, num_trees);

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

void AForestBuilder::UpdateVisibleTrees(FVector const& player_location)
{
	for (auto & tree : trees) {
		auto const tree_location = tree->GetComponentLocation();

		UE_LOG(LogTemp, Display, TEXT("Tree location: (%f,%f,%f)"), tree_location.X, tree_location.Y,
			tree_location.Z);

		auto const distance_vector = tree_location - player_location;

		float const distance = distance_vector.Size();	// get magnitude

		if (distance < render_distance && !tree->IsVisible()) {
			tree->SetVisibility(true);
		}
		else {
			tree->SetVisibility(false);
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

		FString object_name = TEXT("tree");
		object_name.Appendf(TEXT("@(%f,%f,%f)"), loc.X, loc.Y, loc.Z);
		auto tree_obj = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(),
			*object_name);

		if (tree_obj) {

			UStaticMesh* mesh = (UStaticMesh*) StaticLoadObject(UStaticMesh::StaticClass(), nullptr,
				mesh_path);

			if (mesh) {
				tree_obj->SetStaticMesh(mesh);

				UE_LOG(LogTemp, Display, TEXT("SpawN TREE! @ (%f,%f,%f)"), loc.X, loc.Y, loc.Z);
				trees.Add(tree_obj);
				tree_obj->RegisterComponent();
				tree_obj->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

				tree_obj->SetRelativeLocation(loc, false);
				tree_obj->AddLocalRotation(rot, false);
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
