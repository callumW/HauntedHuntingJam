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
