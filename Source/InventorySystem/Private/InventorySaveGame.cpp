// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySaveGame.h"

#include "InventoryListContainer.h"
#include "InventoryItemDefinition.h"
#include "InventorySlotData.h"

void FInventoryItemEntrySaveData::SaveEntry(const FInventoryItemEntry& InItemEntry)
{
	ItemSlotFilter = InItemEntry.ItemSlotFilter;
	
	if (!InItemEntry.IsSlotEmpty())
	{
		ItemDefinition = TSoftObjectPtr<UInventoryItemDefinition>(InItemEntry.ItemDefinition);
		if (const auto CDO = InItemEntry.ItemDefinition->GetSlotDataDefinitionCDO())
		{
			ItemSlotDataSaveGame = CDO->GetSlotDataSaveGame(InItemEntry);
		}
	}
	else
	{
		ItemDefinition = TSoftObjectPtr<UInventoryItemDefinition>();
		ItemSlotDataSaveGame = FInstancedStruct();
	}
}

void FInventoryItemEntrySaveData::LoadEntry(FInventoryItemEntry& InItemEntry, UObject* OuterObject) const
{
	if (!InItemEntry.IsSlotEmpty())
	{
		InItemEntry.EmptySlot();
	}

	InItemEntry.ItemSlotFilter = ItemSlotFilter;
	InItemEntry.ItemDefinition = ItemDefinition.LoadSynchronous();
	if (InItemEntry.ItemDefinition)
	{
		if (const auto CDO = InItemEntry.ItemDefinition->GetSlotDataDefinitionCDO())
		{
			CDO->LoadSlotData(InItemEntry, ItemSlotDataSaveGame, OuterObject);
		}
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
	// Set num.
	const auto SaveDataNum = ItemsSaveData.Num();
	// If item list is more than save data num.
	if (InItemList.ItemList.Num() > SaveDataNum)
	{
		for (int i = InItemList.ItemList.Num() - 1; i >= SaveDataNum; --i)
		{
			InItemList.ItemList.RemoveAt(i);
			InItemList.MarkIndexRemove(i);
		}
	}
	
	for (int i = 0; i < ItemsSaveData.Num(); ++i)
	{
		if (InItemList.ItemList.IsValidIndex(i))
		{
			ItemsSaveData[i].LoadEntry(InItemList.ItemList[i], OuterObject);
			InItemList.MarkIndexChange(i);
		}
		else
		{
			InItemList.ItemList.Add(FInventoryItemEntry());
			ItemsSaveData[i].LoadEntry(InItemList.ItemList[i], OuterObject);
			InItemList.MarkIndexAdd(i);
		}
	}
}
