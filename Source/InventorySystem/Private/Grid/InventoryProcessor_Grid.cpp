// Copyright CrazyDog. All Rights Reserved.

#include "Grid/InventoryProcessor_Grid.h"

#include "InventoryItemDefinition.h"

// --- Shape helpers ----------------------------------------------------------

FItemFragment_GridShape UItemProcessor_Grid::GetItemShape(const UInventoryItemDefinition* ItemDefinition)
{
	return FInventoryGridShapeUtils::GetShape(ItemDefinition);
}

TArray<FIntPoint> UItemProcessor_Grid::GetRotatedCells(const UInventoryItemDefinition* ItemDefinition, EItemGridRotation Rotation)
{
	return FInventoryGridShapeUtils::GetRotatedCells(FInventoryGridShapeUtils::GetShape(ItemDefinition), Rotation);
}

FIntPoint UItemProcessor_Grid::GetRotatedSize(const UInventoryItemDefinition* ItemDefinition, EItemGridRotation Rotation)
{
	const FItemFragment_GridShape Shape = FInventoryGridShapeUtils::GetShape(ItemDefinition);
	return FInventoryGridShapeUtils::GetRotatedSize(Shape.Width, Shape.Height, Rotation);
}

bool UItemProcessor_Grid::CanItemRotate(const UInventoryItemDefinition* ItemDefinition)
{
	return FInventoryGridShapeUtils::GetShape(ItemDefinition).bCanRotate;
}

EItemGridRotation UItemProcessor_Grid::NextRotation(EItemGridRotation Rotation)
{
	return FInventoryGridShapeUtils::NextRotation(Rotation);
}

// --- Queries ----------------------------------------------------------------

bool UItemProcessor_Grid::CanPlaceAt(const FGridInventoryItemList& ItemList, const UInventoryItemDefinition* ItemDefinition, FIntPoint Root, EItemGridRotation Rotation)
{
	return ItemList.CanPlaceAt(ItemDefinition, Root, Rotation, INDEX_NONE);
}

int32 UItemProcessor_Grid::GetEntryIndexAtCell(const FGridInventoryItemList& ItemList, FIntPoint Cell)
{
	return ItemList.GetEntryIndexAtCell(Cell);
}

TArray<FIntPoint> UItemProcessor_Grid::GetEntryWorldCells(const FGridInventoryItemList& ItemList, int32 Index)
{
	if (ItemList.ItemList.IsValidIndex(Index))
	{
		return ItemList.GetWorldCellsForEntry(ItemList.ItemList[Index]);
	}
	return {};
}

bool UItemProcessor_Grid::FindFirstFit(const FGridInventoryItemList& ItemList, const UInventoryItemDefinition* ItemDefinition, FIntPoint& OutRoot, EItemGridRotation& OutRotation)
{
	return ItemList.FindFirstFit(ItemDefinition, OutRoot, OutRotation);
}

// --- Mutations --------------------------------------------------------------

int32 UItemProcessor_Grid::AutoAddItemByDefinition(FGridInventoryItemList& ItemList, UInventoryItemDefinition* ItemDefinition, int32 StackAmount)
{
	return ItemList.AutoAddItemByDefinition(ItemDefinition, StackAmount);
}

int32 UItemProcessor_Grid::AddItemByDefinitionAt(FGridInventoryItemList& ItemList, UInventoryItemDefinition* ItemDefinition, FIntPoint Root, EItemGridRotation Rotation, int32 StackAmount)
{
	return ItemList.AddItemByDefinitionAt(ItemDefinition, Root, Rotation, StackAmount);
}

bool UItemProcessor_Grid::MoveItem(FGridInventoryItemList& ItemList, int32 Index, FIntPoint NewRoot, EItemGridRotation NewRotation)
{
	return ItemList.MoveItem(Index, NewRoot, NewRotation);
}

bool UItemProcessor_Grid::RotateItem(FGridInventoryItemList& ItemList, int32 Index)
{
	return ItemList.RotateItem(Index);
}

bool UItemProcessor_Grid::RemoveItem(FGridInventoryItemList& ItemList, int32 Index, bool bDestroyInstance)
{
	return ItemList.RemoveItem(Index, bDestroyInstance);
}

bool UItemProcessor_Grid::DragDropToGrid(FGridInventoryItemList& FromList, int32 FromIndex, FGridInventoryItemList& ToList, FIntPoint ToRoot, EItemGridRotation ToRotation)
{
	return FromList.DragDropToGrid(FromIndex, ToList, ToRoot, ToRotation);
}

void UItemProcessor_Grid::MarkEntryIndexDirty(FGridInventoryItemList& ItemList, int32 Index)
{
	ItemList.MarkEntryIndexDirty(Index);
}
