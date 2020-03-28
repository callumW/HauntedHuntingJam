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

}

// Called when the game starts or when spawned
void AForestBuilder::BeginPlay()
{
	Super::BeginPlay();

	auto map = m_generator.GenerateForest();

	OutputToBMP(map, FPaths::ProjectDir() + "/trees_");


}

// Called every frame
void AForestBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	size_t const pixels_per_point = 1;
	size_t const num_pixels = static_cast<size_t>(width * height * pixels_per_point);

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
