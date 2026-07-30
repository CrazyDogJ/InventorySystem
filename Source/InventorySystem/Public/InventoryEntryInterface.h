// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryListContainer.h"
#include "UObject/Interface.h"
#include "InventoryEntryInterface.generated.h"

struct FInventoryItemEntry;

/** Everything could be the single item container. */
UINTERFACE()
class UInventoryEntryInterface : public UInterface
{
	GENERATED_BODY()
};

/** Everything could be the single item container. */
class INVENTORYSYSTEM_API IInventoryEntryInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	FInventoryItemEntry GetItemEntry();
	virtual FInventoryItemEntry GetItemEntry_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	void SetItemEntry(FInventoryItemEntry NewEntry);
	virtual void SetItemEntry_Implementation(FInventoryItemEntry NewEntry);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	bool IsItemEntryValid(const int InstanceIndex);
	virtual bool IsItemEntryValid_Implementation(const int InstanceIndex) { return true; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	void OnEntryPickedUp(int Index);
	virtual void OnEntryPickedUp_Implementation(int Index) {}
};
