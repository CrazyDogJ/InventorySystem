// Copyright CrazyDog. All Rights Reserved.

#include "Grid/InventoryGridList.h"

#include "InventoryItemDefinition.h"
#include "InventoryItemInstance.h"
#include "Processors/InventoryProcessor_Stackable.h"

// ---------------------------------------------------------------------------
// Shape resolution
// ---------------------------------------------------------------------------

FItemFragment_GridShape FInventoryGridShapeUtils::GetShape(const UInventoryItemDefinition* ItemDefinition)
{
	if (ItemDefinition)
	{
		if (const FItemFragment_GridShape* Found = ItemDefinition->GetFragmentPtr<FItemFragment_GridShape>())
		{
			return *Found;
		}
	}
	// Default: a single 1x1 cell that cannot be rotated.
	FItemFragment_GridShape Default;
	Default.Width = 1;
	Default.Height = 1;
	Default.bCanRotate = false;
	return Default;
}

// ---------------------------------------------------------------------------
// Geometry / queries
// ---------------------------------------------------------------------------

TArray<FIntPoint> FGridInventoryItemList::GetWorldCells(const UInventoryItemDefinition* ItemDefinition, const FIntPoint& Root, EItemGridRotation Rotation) const
{
	const FItemFragment_GridShape Shape = FInventoryGridShapeUtils::GetShape(ItemDefinition);
	TArray<FIntPoint> Cells = FInventoryGridShapeUtils::GetRotatedCells(Shape, Rotation);
	for (FIntPoint& C : Cells)
	{
		C += Root;
	}
	return Cells;
}

TArray<FIntPoint> FGridInventoryItemList::GetWorldCellsForEntry(const FGridInventoryItemEntry& Entry) const
{
	return GetWorldCells(Entry.ItemDefinition, Entry.RootPosition, Entry.Rotation);
}

bool FGridInventoryItemList::IsWithinBounds(const TArray<FIntPoint>& Cells) const
{
	for (const FIntPoint& C : Cells)
	{
		if (C.X < 0 || C.Y < 0 || C.X >= GridWidth || C.Y >= GridHeight)
		{
			return false;
		}
	}
	return true;
}

bool FGridInventoryItemList::AreCellsFree(const TArray<FIntPoint>& Cells, int32 IgnoreIndex) const
{
	for (const FIntPoint& C : Cells)
	{
		if (const int32* Occupant = OccupancyMap.Find(C))
		{
			if (*Occupant != IgnoreIndex)
			{
				return false;
			}
		}
	}
	return true;
}

bool FGridInventoryItemList::CanPlaceAt(const UInventoryItemDefinition* ItemDefinition, const FIntPoint& Root, EItemGridRotation Rotation, int32 IgnoreIndex) const
{
	if (!ItemDefinition)
	{
		return false;
	}
	const TArray<FIntPoint> Cells = GetWorldCells(ItemDefinition, Root, Rotation);
	if (Cells.Num() == 0)
	{
		return false;
	}
	return IsWithinBounds(Cells) && AreCellsFree(Cells, IgnoreIndex);
}

int32 FGridInventoryItemList::GetEntryIndexAtCell(const FIntPoint& Cell) const
{
	if (const int32* Found = OccupancyMap.Find(Cell))
	{
		return *Found;
	}
	return INDEX_NONE;
}

TArray<int32> FGridInventoryItemList::GetOverlappingIndices(const UInventoryItemDefinition* ItemDefinition, const FIntPoint& Root, EItemGridRotation Rotation, int32 IgnoreIndex) const
{
	TArray<int32> Result;
	const TArray<FIntPoint> Cells = GetWorldCells(ItemDefinition, Root, Rotation);
	for (const FIntPoint& C : Cells)
	{
		if (const int32* Occupant = OccupancyMap.Find(C))
		{
			if (*Occupant != IgnoreIndex)
			{
				Result.AddUnique(*Occupant);
			}
		}
	}
	return Result;
}

bool FGridInventoryItemList::FindFirstFit(const UInventoryItemDefinition* ItemDefinition, FIntPoint& OutRoot, EItemGridRotation& OutRotation) const
{
	if (!ItemDefinition)
	{
		return false;
	}

	const FItemFragment_GridShape Shape = FInventoryGridShapeUtils::GetShape(ItemDefinition);
	const int32 NumRotations = Shape.bCanRotate ? 4 : 1;

	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			for (int32 R = 0; R < NumRotations; ++R)
			{
				const EItemGridRotation Rot = static_cast<EItemGridRotation>(R);
				const FIntPoint Root(X, Y);
				if (CanPlaceAt(ItemDefinition, Root, Rot, INDEX_NONE))
				{
					OutRoot = Root;
					OutRotation = Rot;
					return true;
				}
			}
		}
	}
	return false;
}

void FGridInventoryItemList::RebuildOccupancy()
{
	OccupancyMap.Reset();
	for (int32 Index = 0; Index < ItemList.Num(); ++Index)
	{
		const FGridInventoryItemEntry& Entry = ItemList[Index];
		if (Entry.IsSlotEmpty() || Entry.RootPosition.X < 0 || Entry.RootPosition.Y < 0)
		{
			continue;
		}
		for (const FIntPoint& C : GetWorldCellsForEntry(Entry))
		{
			OccupancyMap.Add(C, Index);
		}
	}
}

// ---------------------------------------------------------------------------
// Mutations (authority)
// ---------------------------------------------------------------------------

void FGridInventoryItemList::MarkEntryIndexDirty(int32 Index)
{
	if (ItemList.IsValidIndex(Index))
	{
		MarkItemDirty(ItemList[Index]);
		OnItemListChange.Broadcast({ Index });
	}
}

int32 FGridInventoryItemList::PlaceEntryAt(FGridInventoryItemEntry& Entry, const FIntPoint& Root, EItemGridRotation Rotation)
{
	if (Entry.IsSlotEmpty() || !CanPlaceAt(Entry.ItemDefinition, Root, Rotation, INDEX_NONE))
	{
		return INDEX_NONE;
	}

	const int32 NewIndex = ItemList.Add(Entry);
	ItemList[NewIndex].RootPosition = Root;
	ItemList[NewIndex].Rotation = Rotation;

	// Re-home the replicated sub-object if needed.
	if (ItemList[NewIndex].ItemInstance)
	{
		ItemList[NewIndex].ItemInstance->ChangeOuter(OuterObject);
	}

	MarkItemDirty(ItemList[NewIndex]);
	RebuildOccupancy();
	OnItemListAdd.Broadcast({ NewIndex });
	return NewIndex;
}

int32 FGridInventoryItemList::AddItemByDefinitionAt(UInventoryItemDefinition* ItemDefinition, const FIntPoint& Root, EItemGridRotation Rotation, int32 StackAmount)
{
	if (!ItemDefinition || !CanPlaceAt(ItemDefinition, Root, Rotation, INDEX_NONE))
	{
		return INDEX_NONE;
	}

	FGridInventoryItemEntry NewEntry(OuterObject, ItemDefinition);
	NewEntry.SetStackAmount(StackAmount);
	return PlaceEntryAt(NewEntry, Root, Rotation);
}

int32 FGridInventoryItemList::AutoAddItemByDefinition(UInventoryItemDefinition* ItemDefinition, int32 StackAmount)
{
	if (!ItemDefinition)
	{
		return INDEX_NONE;
	}

	FIntPoint Root;
	EItemGridRotation Rotation;
	if (!FindFirstFit(ItemDefinition, Root, Rotation))
	{
		return INDEX_NONE;
	}
	return AddItemByDefinitionAt(ItemDefinition, Root, Rotation, StackAmount);
}

bool FGridInventoryItemList::MoveItem(int32 Index, const FIntPoint& NewRoot, EItemGridRotation NewRotation)
{
	if (!ItemList.IsValidIndex(Index) || ItemList[Index].IsSlotEmpty())
	{
		return false;
	}
	if (!CanPlaceAt(ItemList[Index].ItemDefinition, NewRoot, NewRotation, /*IgnoreIndex*/ Index))
	{
		return false;
	}

	ItemList[Index].RootPosition = NewRoot;
	ItemList[Index].Rotation = NewRotation;
	MarkItemDirty(ItemList[Index]);
	RebuildOccupancy();
	OnItemListChange.Broadcast({ Index });
	return true;
}

bool FGridInventoryItemList::RotateItem(int32 Index)
{
	if (!ItemList.IsValidIndex(Index) || ItemList[Index].IsSlotEmpty())
	{
		return false;
	}

	const FItemFragment_GridShape Shape = FInventoryGridShapeUtils::GetShape(ItemList[Index].ItemDefinition);
	if (!Shape.bCanRotate)
	{
		return false;
	}

	const EItemGridRotation NewRotation = FInventoryGridShapeUtils::NextRotation(ItemList[Index].Rotation);
	return MoveItem(Index, ItemList[Index].RootPosition, NewRotation);
}

bool FGridInventoryItemList::RemoveItem(int32 Index, bool bDestroyInstance)
{
	if (!ItemList.IsValidIndex(Index))
	{
		return false;
	}

	OnItemListRemove.Broadcast({ Index });
	ItemList[Index].EmptySlot(bDestroyInstance);
	ItemList.RemoveAt(Index, EAllowShrinking::No);
	ItemList.Shrink();
	MarkArrayDirty();
	RebuildOccupancy();
	return true;
}

bool FGridInventoryItemList::DragDropToGrid(int32 FromIndex, FGridInventoryItemList& ToList, const FIntPoint& ToRoot, EItemGridRotation ToRotation)
{
	if (!ItemList.IsValidIndex(FromIndex) || ItemList[FromIndex].IsSlotEmpty())
	{
		return false;
	}

	// Same grid: this is just a move (or rotate-in-place).
	if (&ToList == this)
	{
		return MoveItem(FromIndex, ToRoot, ToRotation);
	}

	UInventoryItemDefinition* MovingDef = ItemList[FromIndex].ItemDefinition;

	// Slot filter on the destination (uses the moved item's slot filter contract).
	if (!ToList.IsWithinBounds(ToList.GetWorldCells(MovingDef, ToRoot, ToRotation)))
	{
		return false;
	}

	const TArray<int32> Overlaps = ToList.GetOverlappingIndices(MovingDef, ToRoot, ToRotation, INDEX_NONE);

	// 1. Free space on the destination -> straight transfer.
	if (Overlaps.Num() == 0)
	{
		FGridInventoryItemEntry Moved;
		Moved.CopyGridFrom(ItemList[FromIndex]);

		const int32 NewIndex = ToList.PlaceEntryAt(Moved, ToRoot, ToRotation);
		if (NewIndex == INDEX_NONE)
		{
			return false;
		}

		// Remove from source WITHOUT destroying the instance (it now lives in ToList).
		RemoveItem(FromIndex, /*bDestroyInstance*/ false);
		return true;
	}

	// 2. Exactly one item in the way -> attempt a swap into the mover's old footprint.
	if (Overlaps.Num() == 1)
	{
		const int32 OtherIndex = Overlaps[0];
		const FGridInventoryItemEntry& Other = ToList.ItemList[OtherIndex];

		const FIntPoint OldRoot = ItemList[FromIndex].RootPosition;
		const EItemGridRotation OldRot = ItemList[FromIndex].Rotation;

		// Does the displaced item fit where the mover used to be? (ignore the mover itself)
		if (CanPlaceAt(Other.ItemDefinition, OldRoot, Other.Rotation, FromIndex) &&
			ToList.CanPlaceAt(MovingDef, ToRoot, ToRotation, OtherIndex))
		{
			FGridInventoryItemEntry MovedCopy;
			MovedCopy.CopyGridFrom(ItemList[FromIndex]);
			FGridInventoryItemEntry OtherCopy;
			OtherCopy.CopyGridFrom(Other);

			// Overwrite source slot with the displaced item at the old footprint.
			ItemList[FromIndex].CopyGridFrom(OtherCopy);
			ItemList[FromIndex].RootPosition = OldRoot;
			ItemList[FromIndex].Rotation = OtherCopy.Rotation;
			if (ItemList[FromIndex].ItemInstance)
			{
				ItemList[FromIndex].ItemInstance->ChangeOuter(OuterObject);
			}

			// Overwrite destination slot with the moved item at the target footprint.
			ToList.ItemList[OtherIndex].CopyGridFrom(MovedCopy);
			ToList.ItemList[OtherIndex].RootPosition = ToRoot;
			ToList.ItemList[OtherIndex].Rotation = ToRotation;
			if (ToList.ItemList[OtherIndex].ItemInstance)
			{
				ToList.ItemList[OtherIndex].ItemInstance->ChangeOuter(ToList.OuterObject);
			}

			MarkEntryIndexDirty(FromIndex);
			ToList.MarkEntryIndexDirty(OtherIndex);
			RebuildOccupancy();
			ToList.RebuildOccupancy();
			return true;
		}
	}

	// Blocked by multiple items, or the swap candidate does not fit.
	return false;
}

// ---------------------------------------------------------------------------
// Replication callbacks (clients rebuild the spatial index, then notify UI)
// ---------------------------------------------------------------------------

void FGridInventoryItemList::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	TArray<int> Indices;
	Indices.Append(RemovedIndices);
	OnItemListRemove.Broadcast(Indices);
}

void FGridInventoryItemList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	RebuildOccupancy();
	TArray<int> Indices;
	Indices.Append(AddedIndices);
	OnItemListAdd.Broadcast(Indices);
}

void FGridInventoryItemList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	RebuildOccupancy();
	TArray<int> Indices;
	Indices.Append(ChangedIndices);
	OnItemListChange.Broadcast(Indices);
}

bool FGridInventoryItemList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FGridInventoryItemEntry, FGridInventoryItemList>(ItemList, DeltaParams, *this);
}
