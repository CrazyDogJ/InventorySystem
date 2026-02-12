// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/TextureThumbnailRenderer.h"
#include "ItemDefThumbnailRenderer.generated.h"

UCLASS()
class INVENTORYSYSTEMEDITOR_API UItemDefThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_BODY()

public:
	/** Checks whether the asset can be visualized using this thumbnail renderer.
	* will fallback to default thumbnail if this returns false. */
	virtual bool CanVisualizeAsset(UObject* Object) override;

	/** To calculate the size of the thumbnail to display, usually should match the textures size*/
	virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override;

	/** This actually draws the thumbnail. */
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, bool bAdditionalViewFamily) override;

private:
	static UTexture2D* GetThumbnailTextureFromObject(UObject* Object);
};
