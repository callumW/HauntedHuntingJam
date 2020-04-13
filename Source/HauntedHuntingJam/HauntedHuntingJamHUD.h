// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/Texture2D.h"
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

	void DisplayText(FString const& text);

	void DisplayReadableTexture(UTexture2D* tex);
private:

	void DrawWoodCount();
	void DrawFireLevel();
	void DrawReadableText();

	/** Crosshair asset pointer */
	UTexture2D* CrosshairTex;

	UTexture2D* readable_texture;

	uint32 wood_count = 0;

	float fire_level = 1.0f;

	bool draw_readable_mode = false;
	FString readable_text;

};
