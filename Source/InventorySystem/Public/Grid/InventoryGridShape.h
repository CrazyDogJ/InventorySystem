// Copyright CrazyDog. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryGridShape.generated.h"

class UInventoryItemDefinition;

/**
 * Grid rotation, in 90-degree steps. Stored as a uint8 so it replicates cheaply
 * inside a fast-array entry. The integer value is the number of clockwise 90 turns.
 */
UENUM(BlueprintType)
enum class EItemGridRotation : uint8
{
	Rot_0	= 0	UMETA(DisplayName = "0"),
	Rot_90	= 1	UMETA(DisplayName = "90"),
	Rot_180	= 2	UMETA(DisplayName = "180"),
	Rot_270	= 3	UMETA(DisplayName = "270"),
};

/**
 * Grid shape fragment.
 *
 * Add this to UInventoryItemDefinition::DefaultFragments to make an item occupy a
 * grid footprint. Rectangular items only need Width/Height. Irregular shapes
 * (L / T / cross ...) use CellMask: a row-major bool grid of size Width*Height
 * where true means the cell is occupied. Leaving CellMask empty means "full
 * Width*Height rectangle".
 *
 * Local space: (0,0) is the top-left cell, +X goes right, +Y goes down.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemFragment_GridShape
{
	GENERATED_BODY()

	/** Footprint width (cells) at rotation 0. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "1"), Category = "Grid Shape")
	int32 Width = 1;

	/** Footprint height (cells) at rotation 0. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "1"), Category = "Grid Shape")
	int32 Height = 1;

	/**
	 * Row-major occupancy mask, size should be Width*Height.
	 * Empty => the whole Width*Height rectangle is occupied.
	 * Use this for non-rectangular shapes.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grid Shape")
	TArray<bool> CellMask;

	/** Whether the player is allowed to rotate this item in the grid. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grid Shape")
	bool bCanRotate = true;

	/** Returns true if the local cell (X,Y) is part of the footprint. */
	bool IsCellOccupied(int32 X, int32 Y) const
	{
		if (X < 0 || Y < 0 || X >= Width || Y >= Height)
		{
			return false;
		}
		if (CellMask.Num() == 0)
		{
			return true; // full rectangle
		}
		const int32 Index = Y * Width + X;
		return CellMask.IsValidIndex(Index) ? CellMask[Index] : false;
	}

	/** Occupied cells at rotation 0, anchored at local origin (0,0). */
	TArray<FIntPoint> GetLocalCells() const
	{
		TArray<FIntPoint> Cells;
		Cells.Reserve(Width * Height);
		for (int32 Y = 0; Y < Height; ++Y)
		{
			for (int32 X = 0; X < Width; ++X)
			{
				if (IsCellOccupied(X, Y))
				{
					Cells.Emplace(X, Y);
				}
			}
		}
		return Cells;
	}
};

/**
 * Pure, side-effect-free helpers for transforming a shape by rotation.
 * Kept separate from the fragment so they can be reused without a definition.
 */
struct INVENTORYSYSTEM_API FInventoryGridShapeUtils
{
	/** Bounding size after rotation (90/270 swap width and height). */
	static FIntPoint GetRotatedSize(int32 Width, int32 Height, EItemGridRotation Rotation)
	{
		switch (Rotation)
		{
		case EItemGridRotation::Rot_90:
		case EItemGridRotation::Rot_270:
			return FIntPoint(Height, Width);
		default:
			return FIntPoint(Width, Height);
		}
	}

	/** Rotate a single local cell inside a Width*Height bounding box. Result stays anchored at origin. */
	static FIntPoint RotateCell(const FIntPoint& Cell, int32 Width, int32 Height, EItemGridRotation Rotation)
	{
		switch (Rotation)
		{
		case EItemGridRotation::Rot_90:	 return FIntPoint(Height - 1 - Cell.Y, Cell.X);
		case EItemGridRotation::Rot_180: return FIntPoint(Width - 1 - Cell.X, Height - 1 - Cell.Y);
		case EItemGridRotation::Rot_270: return FIntPoint(Cell.Y, Width - 1 - Cell.X);
		default:						 return Cell;
		}
	}

	/** Occupied cells after rotation, anchored at origin (0,0). */
	static TArray<FIntPoint> GetRotatedCells(const FItemFragment_GridShape& Shape, EItemGridRotation Rotation)
	{
		TArray<FIntPoint> Out = Shape.GetLocalCells();
		if (Rotation != EItemGridRotation::Rot_0)
		{
			for (FIntPoint& C : Out)
			{
				C = RotateCell(C, Shape.Width, Shape.Height, Rotation);
			}
		}
		return Out;
	}

	/** Next clockwise rotation. */
	static EItemGridRotation NextRotation(EItemGridRotation Rotation)
	{
		return static_cast<EItemGridRotation>((static_cast<uint8>(Rotation) + 1) & 0x3);
	}

	/** Resolve the shape fragment from a definition. Falls back to a 1x1 shape if none is present. */
	static FItemFragment_GridShape GetShape(const UInventoryItemDefinition* ItemDefinition);
};
