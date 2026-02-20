// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryEntryInterface.h"


// Add default functionality here for any IInventoryEntryInterface functions that are not pure virtual.
FInventoryItemEntry IInventoryEntryInterface::GetItemEntry_Implementation()
{
	return FInventoryItemEntry();
}

void IInventoryEntryInterface::SetItemEntry_Implementation(FInventoryItemEntry NewEntry)
{
}
