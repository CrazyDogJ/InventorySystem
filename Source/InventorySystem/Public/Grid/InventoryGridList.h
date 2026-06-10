// Copyright CrazyDog. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryListContainer.h"
#include "InventoryUtility.h"
#include "Grid/InventoryGridShape.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryGridList.generated.h"

class UInventoryItemDefinition;

/**
 * A single placed item in a grid inventory.
 *
 * Subclasses the slot-based FInventoryItemEntry (so it keeps ItemDefinition,
 * ItemInstance, stacking and slot-filter behaviour) and adds the two pieces of
 * data a grid needs: the top-left anchor cell and the rotation. Both are plain
 * replicated UPROPERTYs, so they ride along inside the fast-array delta.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FGridInventoryItemEntry : public FInventoryItemEntry
{
	GENERATED_BODY()

	FGridInventoryItemEntry() {}
	FGridInventoryItemEntry(UObject* OuterObject, UInventoryItemDefinition* InItemDefinition)
		: FInventoryItemEntry(OuterObject, InItemDefinition) {}

	/** Top-left anchor cell of the footprint. (-1,-1) means "not placed yet". */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grid")
	FIntPoint RootPosition = FIntPoint(-1, -1);

	/** Current rotation (clockwise, 90-degree steps). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grid")
	EItemGridRotation Rotation = EItemGridRotation::Rot_0;

	void CopyGridFrom(const FGridInventoryItemEntry& Other)
	{
		CopyFrom(Other);
		RootPosition = Other.RootPosition;
		Rotation = Other.Rotation;
	}
};

/**
 * A grid (Tarkov-style) inventory container.
 *
 * Stores arbitrarily-shaped, rotatable items addressed by cell position rather
 * than by a fixed slot index. Replicates incrementally through the same
 * FastArraySerializer machinery the slot-based list uses.
 *
 * The OccupancyMap is a transient acceleration structure (cell -> entry index).
 * It is never replicated; both server and clients rebuild it from the entries,
 * so placement queries stay O(footprint) instead of O(items).
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FGridInventoryItemList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grid")
	TArray<FGridInventoryItemEntry> ItemList;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "1"), Category = "Grid")
	int32 GridWidth = 10;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "1"), Category = "Grid")
	int32 GridHeight = 10;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, NotReplicated, Category = "Grid")
	UObject* OuterObject = nullptr;

	/** Transient cell -> entry-index lookup. Rebuilt from ItemList, never replicated. */
	TMap<FIntPoint, int32> OccupancyMap;

	// --- Geometry / queries -------------------------------------------------

	/** Absolute cells an entry occupies given its current root + rotation + shape. */
	TArray<FIntPoint> GetWorldCellsForEntry(const FGridInventoryItemEntry& Entry) const;

	/** Absolute cells a hypothetical placement would occupy. */
	TArray<FIntPoint> GetWorldCells(const UInventoryItemDefinition* ItemDefinition, const FIntPoint& Root, EItemGridRotation Rotation) const;

	bool IsWithinBounds(const TArray<FIntPoint>& Cells) const;

	/** All cells free (optionally ignoring one entry, e.g. the item being moved/rotated)? */
	bool AreCellsFree(const TArray<FIntPoint>& Cells, int32 IgnoreIndex = INDEX_NONE) const;

	/** Full placement test: in-bounds AND not colliding. */
	bool CanPlaceAt(const UInventoryItemDefinition* ItemDefinition, const FIntPoint& Root, EItemGridRotation Rotation, int32 IgnoreIndex = INDEX_NONE) const;

	/** Entry index occupying a cell, or INDEX_NONE. */
	int32 GetEntryIndexAtCell(const FIntPoint& Cell) const;

	/** Distinct entry indices overlapped by a hypothetical placement (excludes IgnoreIndex). */
	TArray<int32> GetOverlappingIndices(const UInventoryItemDefinition* ItemDefinition, const FIntPoint& Root, EItemGridRotation Rotation, int32 IgnoreIndex = INDEX_NONE) const;

	/** Scan for the first position/rotation that fits. Honours bCanRotate. */
	bool FindFirstFit(const UInventoryItemDefinition* ItemDefinition, FIntPoint& OutRoot, EItemGridRotation& OutRotation) const;

	void RebuildOccupancy();

	// --- Mutations (call on authority) -------------------------------------

	/** Place an already-built entry at an explicit position. Returns the new index or INDEX_NONE. */
	int32 PlaceEntryAt(FGridInventoryItemEntry& Entry, const FIntPoint& Root, EItemGridRotation Rotation);

	/** Build + place an item by definition at the first free fit. */
	int32 AutoAddItemByDefinition(UInventoryItemDefinition* ItemDefinition, int32 StackAmount = 1);

	/** Build + place an item by definition at an explicit position. */
	int32 AddItemByDefinitionAt(UInventoryItemDefinition* ItemDefinition, const FIntPoint& Root, EItemGridRotation Rotation, int32 StackAmount = 1);

	/** Move an existing entry to a new position/rotation within this grid. */
	bool MoveItem(int32 Index, const FIntPoint& NewRoot, EItemGridRotation NewRotation);

	/** Rotate an entry in place (around its current root). No-op if it would not fit. */
	bool RotateItem(int32 Index);

	/** Remove an entry entirely (frees its cells). */
	bool RemoveItem(int32 Index, bool bDestroyInstance = true);

	/**
	 * Drop an entry from this grid onto another grid (or the same one) at a target
	 * position. If exactly one item is in the way and it can be relocated to the
	 * mover's old footprint, the two are swapped (Tarkov-style).
	 */
	bool DragDropToGrid(int32 FromIndex, FGridInventoryItemList& ToList, const FIntPoint& ToRoot, EItemGridRotation ToRotation);

	// --- Fast array plumbing ------------------------------------------------

	void MarkEntryIndexDirty(int32 Index);

	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);

	FOnFastArraySerializerEvent OnItemListRemove;
	FOnFastArraySerializerEvent OnItemListAdd;
	FOnFastArraySerializerEvent OnItemListChange;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);
};

template<> struct TStructOpsTypeTraits<FGridInventoryItemList> : public TStructOpsTypeTraitsBase2<FGridInventoryItemList>
{
	enum { WithNetDeltaSerializer = true };
};
