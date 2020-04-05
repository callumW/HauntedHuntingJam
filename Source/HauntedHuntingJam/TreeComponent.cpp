// Fill out your copyright notice in the Description page of Project Settings.


#include "TreeComponent.h"


bool UTreeComponent::Initialize(TCHAR const* mesh_path)
{
    UStaticMesh* mesh = (UStaticMesh*) StaticLoadObject(UStaticMesh::StaticClass(), nullptr,
        mesh_path);

    if (mesh) {
        SetStaticMesh(mesh);
        return true;
    }
    return false;
}

bool UTreeComponent::OnHit()
{
    --life_points;
    UE_LOG(LogTemp, Display, TEXT("HitTree, %u life left!"), life_points);
    if (life_points <= 0) {
        return true;
    }
    else {
        return false;
    }
}
