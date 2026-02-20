// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FastArraySerializers/InventoryListContainer.h"
#include "UObject/Interface.h"
#include "InventoryEntryInterface.generated.h"

struct FInventoryItemEntry;

UINTERFACE()
class UInventoryEntryInterface : public UInterface
{
	GENERATED_BODY()
};

class INVENTORYSYSTEM_API IInventoryEntryInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FInventoryItemEntry GetItemEntry();
	virtual FInventoryItemEntry GetItemEntry_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetItemEntry(FInventoryItemEntry NewEntry);
	virtual void SetItemEntry_Implementation(FInventoryItemEntry NewEntry);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsItemEntryValid(const int InstanceIndex);
	virtual bool IsItemEntryValid_Implementation(const int InstanceIndex) { return true; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnEntryPickedUp(int Index);
	virtual void OnEntryPickedUp_Implementation(int Index) {}
};
