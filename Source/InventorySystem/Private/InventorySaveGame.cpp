// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySaveGame.h"

#include "FastArraySerializers/InventoryListContainer.h"
#include "InventoryItemInstance.h"
#include "InventoryItemDefinition.h"

void FInventoryItemEntrySaveData::SaveEntry(const FInventoryItemEntry& InItemEntry)
{
	if (!InItemEntry.IsSlotEmpty())
	{
		ItemDefinition = TSoftObjectPtr<UInventoryItemDefinition>(InItemEntry.ItemInstance->ItemDefinition);
		InItemEntry.ItemInstance->GetSaveData(ItemInstanceData);
	}
}

void FInventoryItemEntrySaveData::LoadEntry(FInventoryItemEntry& InItemEntry, UObject* OuterObject) const
{
	if (ItemInstanceData.IsEmpty()) return;
	
	if (!InItemEntry.IsSlotEmpty())
	{
		InItemEntry.EmptySlot();
	}

	InItemEntry.ItemInstance = UInventoryItemInstance::NewItemInstance(OuterObject, ItemDefinition.LoadSynchronous());
	InItemEntry.ItemInstance->LoadSaveData(ItemInstanceData);
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
