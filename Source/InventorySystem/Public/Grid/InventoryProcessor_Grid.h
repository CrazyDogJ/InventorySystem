// Copyright CrazyDog. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Grid/InventoryGridList.h"
#include "Grid/InventoryGridShape.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryProcessor_Grid.generated.h"

class UInventoryItemDefinition;

/**
 * Blueprint entry points for the grid inventory.
 *
 * Same role as UItemProcessor_Stackable for the slot-based list: thin, static
 * wrappers around FGridInventoryItemList so the spatial logic is callable from
 * Blueprints. All mutating calls are BlueprintAuthorityOnly.
 */
UCLASS()
class INVENTORYSYSTEM_API UItemProcessor_Grid : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// --- Shape helpers ------------------------------------------------------

	/** Footprint shape for a definition (1x1 fallback if it has no grid fragment). */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Grid")
	static FItemFragment_GridShape GetItemShape(const UInventoryItemDefinition* ItemDefinition);

	/** Occupied cells (relative to origin) at a given rotation. Good for drawing ghosts/highlights. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Grid")
	static TArray<FIntPoint> GetRotatedCells(const UInventoryItemDefinition* ItemDefinition, EItemGridRotation Rotation);

	/** Bounding size of a definition's footprint at a given rotation. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Grid")
	static FIntPoint GetRotatedSize(const UInventoryItemDefinition* ItemDefinition, EItemGridRotation Rotation);

	/** Whether a definition's item is allowed to rotate. */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Grid")
	static bool CanItemRotate(const UInventoryItemDefinition* ItemDefinition);

	UFUNCTION(BlueprintPure, Category = "Inventory System|Grid")
	static EItemGridRotation NextRotation(EItemGridRotation Rotation);

	// --- Queries ------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Inventory System|Grid")
	static bool CanPlaceAt(const FGridInventoryItemList& ItemList, const UInventoryItemDefinition* ItemDefinition, FIntPoint Root, EItemGridRotation Rotation);

	UFUNCTION(BlueprintPure, Category = "Inventory System|Grid")
	static int32 GetEntryIndexAtCell(const FGridInventoryItemList& ItemList, FIntPoint Cell);

	/** Absolute cells the placed entry currently occupies (for UI). */
	UFUNCTION(BlueprintPure, Category = "Inventory System|Grid")
	static TArray<FIntPoint> GetEntryWorldCells(const FGridInventoryItemList& ItemList, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|Grid")
	static bool FindFirstFit(const FGridInventoryItemList& ItemList, const UInventoryItemDefinition* ItemDefinition, FIntPoint& OutRoot, EItemGridRotation& OutRotation);

	// --- Mutations (authority) ---------------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Grid")
	static int32 AutoAddItemByDefinition(UPARAM(ref) FGridInventoryItemList& ItemList, UInventoryItemDefinition* ItemDefinition, int32 StackAmount = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Grid")
	static int32 AddItemByDefinitionAt(UPARAM(ref) FGridInventoryItemList& ItemList, UInventoryItemDefinition* ItemDefinition, FIntPoint Root, EItemGridRotation Rotation, int32 StackAmount = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Grid")
	static bool MoveItem(UPARAM(ref) FGridInventoryItemList& ItemList, int32 Index, FIntPoint NewRoot, EItemGridRotation NewRotation);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Grid")
	static bool RotateItem(UPARAM(ref) FGridInventoryItemList& ItemList, int32 Index);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Grid")
	static bool RemoveItem(UPARAM(ref) FGridInventoryItemList& ItemList, int32 Index, bool bDestroyInstance = true);

	/** Drag an item from one grid to another (or the same), with Tarkov-style single-item swap. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Grid")
	static bool DragDropToGrid(UPARAM(ref) FGridInventoryItemList& FromList, int32 FromIndex, UPARAM(ref) FGridInventoryItemList& ToList, FIntPoint ToRoot, EItemGridRotation ToRotation);

	/** Call after directly editing an entry in the list (mirrors MarkIndexDirty on the slot list). */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Grid")
	static void MarkEntryIndexDirty(UPARAM(ref) FGridInventoryItemList& ItemList, int32 Index);
};
