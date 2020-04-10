// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "HauntedHuntingJamHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"

AHauntedHuntingJamHUD::AHauntedHuntingJamHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshairTexObj.Object;
}

void AHauntedHuntingJamHUD::UpdateFireLevel(float level)
{
	fire_level = level;
}

void AHauntedHuntingJamHUD::DrawFireLevel()
{
	FString text{TEXT("Fire level:")};
	text.Appendf(TEXT(" %f"), fire_level * 100.0f);

	DrawText(text, FLinearColor::Red, Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f + 10.f,
		nullptr, 1.0f, false);
}

void AHauntedHuntingJamHUD::DrawWoodCount()
{
	FString text{TEXT("Wood Count:")};
	text.Appendf(TEXT(" %u"), wood_count);

	DrawText(text, FLinearColor::Red, Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f, nullptr, 1.0f, false);

}

void AHauntedHuntingJamHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X),
										   (Center.Y + 20.0f));

	// draw the crosshair
	FCanvasTileItem TileItem( CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem( TileItem );

	DrawWoodCount();
	DrawFireLevel();
}

void AHauntedHuntingJamHUD::UpdateWoodCount(uint32 new_wood_count)
{
	wood_count = new_wood_count;
}
