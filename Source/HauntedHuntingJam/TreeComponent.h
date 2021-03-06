// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "TreeComponent.generated.h"

/**
 *
 */
UCLASS()
class HAUNTEDHUNTINGJAM_API UTreeComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

	unsigned life_points = 5;

public:
	bool Initialize(TCHAR const* mesh_path);

	bool OnHit();
};
