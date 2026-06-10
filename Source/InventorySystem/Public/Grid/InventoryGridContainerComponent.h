// Copyright CrazyDog. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryContainerComponent.h"
#include "Grid/InventoryGridList.h"
#include "InventoryGridContainerComponent.generated.h"

/**
 * A container component for grid (Tarkov-style) inventories.
 *
 * Reuses everything the base UInventoryContainerComponent provides (replication
 * setup, registered sub-object list, streaming save hooks) and swaps the flat
 * slot list for a spatial FGridInventoryItemList.
 *
 * The inherited ItemList is left unused/empty here; all grid items live in
 * GridItemList. If you want a hybrid container (e.g. equipment slots + a grid
 * stash) you can use both at once.
 */
UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent), Blueprintable)
class INVENTORYSYSTEM_API UGridInventoryContainerComponent : public UInventoryContainerComponent
{
	GENERATED_BODY()

public:
	UGridInventoryContainerComponent();

	/** The spatial item list. Replicated incrementally via the fast-array serializer. */
	UPROPERTY(BlueprintReadWrite, Replicated, VisibleAnywhere, Transient, Category = "Inventory System|Grid")
	FGridInventoryItemList GridItemList;

	/** Grid size applied on BeginPlay (authority). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "1"), Category = "Inventory System|Grid")
	int32 InitGridWidth = 10;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "1"), Category = "Inventory System|Grid")
	int32 InitGridHeight = 10;

	/** Convenience: bind a Blueprint event to grid add/remove/change for UI refresh. */
	UFUNCTION(BlueprintCallable, Category = "Inventory System|Grid")
	void BindGridAddEvent(const FOnFastArraySerializerStaticEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|Grid")
	void BindGridRemoveEvent(const FOnFastArraySerializerStaticEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|Grid")
	void BindGridChangeEvent(const FOnFastArraySerializerStaticEvent& Event);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
};
