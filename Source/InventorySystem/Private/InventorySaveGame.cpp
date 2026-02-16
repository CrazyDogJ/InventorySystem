// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySaveGame.h"

#include "FastArraySerializers/InventoryListContainer.h"
#include "InventoryItemInstance.h"
#include "InventoryItemDefinition.h"

void FInventoryItemEntrySaveData::SaveEntry(const FInventoryItemEntry& InItemEntry)
{
	if (!InItemEntry.IsSlotEmpty())
	{
		ItemDefinition = TSoftObjectPtr<UInventoryItemDefinition>(InItemEntry.ItemDefinition);
		ItemStack = InItemEntry.ItemStack;
		if (InItemEntry.ItemInstance)
		{
			InItemEntry.ItemInstance->GetSaveData(ItemInstanceData);
		}
	}
	else
	{
		ItemDefinition = TSoftObjectPtr<UInventoryItemDefinition>();
		ItemStack = 0;
		ItemInstanceData = TArray<uint8>();
	}
}

void FInventoryItemEntrySaveData::LoadEntry(FInventoryItemEntry& InItemEntry, UObject* OuterObject) const
{
	if (!InItemEntry.IsSlotEmpty())
	{
		InItemEntry.EmptySlot();
	}

	InItemEntry.ItemDefinition = ItemDefinition.LoadSynchronous();
	InItemEntry.ItemStack = ItemStack;
	if (!ItemInstanceData.IsEmpty())
	{
		InItemEntry.ItemInstance = UInventoryItemInstance::NewItemInstance(OuterObject, InItemEntry.ItemDefinition);
		InItemEntry.ItemInstance->LoadSaveData(ItemInstanceData);
	}
}

void FInventoryItemListSaveData::SaveList(const FInventoryItemList& InItemList)
{
	for (const FInventoryItemEntry& Itr : InItemList.ItemList)
	{
		FInventoryItemEntrySaveData NewEntryData;
		NewEntryData.SaveEntry(Itr);
		ItemsSaveData.Add(NewEntryData);
	}
}

void FInventoryItemListSaveData::LoadList(FInventoryItemList& InItemList, UObject* OuterObject) const
{
	InItemList.Clear();
	InItemList.AddEmptySlots(ItemsSaveData.Num());
	TArray<int32> Indices;
	for (int i = 0; i < ItemsSaveData.Num(); ++i)
	{
		ItemsSaveData[i].LoadEntry(InItemList.ItemList[i], OuterObject);
		Indices.Add(i);
	}

	InItemList.MarkIndexDirty(Indices);
}
