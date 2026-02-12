// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDefThumbnailRenderer.h"

#include "InventoryItemDefinition.h"

bool UItemDefThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	return IsValid(GetThumbnailTextureFromObject(Object));
}

void UItemDefThumbnailRenderer::GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const
{
	Super::GetThumbnailSize(GetThumbnailTextureFromObject(Object), Zoom, OutWidth, OutHeight);
}

void UItemDefThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height,
	FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	Super::Draw(GetThumbnailTextureFromObject(Object), X, Y, Width, Height, RenderTarget, Canvas, bAdditionalViewFamily);
}

UTexture2D* UItemDefThumbnailRenderer::GetThumbnailTextureFromObject(UObject* Object)
{
	const UInventoryItemDefinition* ItemDef = Cast<UInventoryItemDefinition>(Object);
	if (!ItemDef)
	{
		return nullptr;
	}

	if (const auto SlateBrush = ItemDef->GetFragmentPtr<FSlateBrush>())
	{
		if (UTexture2D* SlateBrushTexture = Cast<UTexture2D>(SlateBrush->GetResourceObject()))
		{
			SlateBrushTexture->BlockOnAnyAsyncBuild();
			return SlateBrushTexture;
		}
	}
 
	return nullptr;
}
