// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HauntedHuntingJamHUD.generated.h"

UCLASS()
class AHauntedHuntingJamHUD : public AHUD
{
	GENERATED_BODY()

public:
	AHauntedHuntingJamHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	void UpdateWoodCount(uint32 new_wood_count);
	void UpdateFireLevel(float level);
private:

	void DrawWoodCount();
	void DrawFireLevel();

	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

	uint32 wood_count = 0;

	float fire_level = 1.0f;

};
