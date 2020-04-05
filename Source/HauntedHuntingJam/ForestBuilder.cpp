// Fill out your copyright notice in the Description page of Project Settings.

#include "ForestBuilder.h"

#include <array>

// Sets default values
AForestBuilder::AForestBuilder()
{
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

bool AForestBuilder::HitTree(UTreeComponent* tree)
{
	if (tree == nullptr) {
		return false;
	}

	size_t i = trees.Find(tree);
	if (i != INDEX_NONE) {
		if (tree->OnHit()) {
			trees.RemoveAt(i);
			tree->DestroyComponent();
			return true;
		}
	}
	return false;
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

		auto tree_obj = NewObject<UTreeComponent>(this, UTreeComponent::StaticClass());

		if (tree_obj) {

			tree_obj->Initialize(mesh_path);

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
			UE_LOG(LogTemp, Warning, TEXT("Failed to create tree!"));
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("No Room component!"));
	}
}
