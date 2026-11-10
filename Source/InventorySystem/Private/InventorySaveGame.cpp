// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySaveGame.h"

#include "InventoryListContainer.h"
#include "InventoryItemDefinition.h"
#include "InventorySlotData.h"

void FInventoryItemEntrySaveData::SaveEntry(const FInventoryItemEntry& InItemEntry)
{
	ItemSlotFilter = InItemEntry.ItemSlotFilter;
	
	// 修复：残留槽数据不代表物品定义仍有效，重用存档结构时丢弃旧数据。
	ItemSlotDataSaveGame.Reset();
	if (IsValid(InItemEntry.ItemDefinition))
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
	// 修复：重复保存到同一结构时替换快照，避免条目无限累加。
	ItemsSaveData.Reset(InItemList.ItemList.Num());
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
		// 修复：统一使用列表删除入口，使客户端和界面及时释放已移除的槽位。
		TArray<int32> Removed;
		for (int32 Index = InItemList.ItemList.Num() - 1; Index >= SaveDataNum; --Index) Removed.Add(Index);
		InItemList.RemoveSlots(MoveTemp(Removed));
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
