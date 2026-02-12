// Fill out your copyright notice in the Description page of Project Settings.

#include "FastArraySerializers/InventoryListContainer.h"
#include "InventoryItemDefinition.h"

FInventoryItemEntry::FInventoryItemEntry(UInventoryItemDefinition* InItemDefinition)
{
	ItemDefinition = InItemDefinition;
	ItemStack = 1;
}

bool FInventoryItemEntry::IsSlotEmpty() const
{
	return ItemDefinition == nullptr;
}

void FInventoryItemEntry::EmptySlot()
{
	ItemDefinition = nullptr;
	ItemStack = 0;
	if (ItemInstance)
	{
		ItemInstance->ConditionalBeginDestroy();
	}
	ItemInstance = nullptr;
}

void FInventoryItemList::EmptySlotByIndex(TArray<int32> Indices, bool bDirty)
{
	for (const int Index : Indices)
	{
		if (ItemList.IsValidIndex(Index))
		{
			ItemList[Index].EmptySlot();
		}
	}
	
	if (bDirty)
	{
		MarkIndexDirty(Indices);
	}
}

void FInventoryItemList::Clear()
{
	TArray<int32> RemovedIndices;
	for (int i = 0; i < ItemList.Num(); ++i)
	{
		ItemList[i].EmptySlot();
		RemovedIndices.Add(i);
	}

	OnItemListRemove.Broadcast(RemovedIndices);
	ItemList.Empty();
	MarkArrayDirty();
}

void FInventoryItemList::AddEmptySlots(const int32 AddAmount)
{
	TArray<int32> AddIndices;
	for (int i = 0; i < AddAmount; ++i)
	{
		const auto OutIndex = ItemList.Add(FInventoryItemEntry());
		AddIndices.Add(OutIndex);
		MarkItemDirty(ItemList[OutIndex]);
	}

	OnItemListAdd.Broadcast(AddIndices);
}

void FInventoryItemList::RemoveSlots(TArray<int32> Indices)
{
	TArray<int32> RemovedIndices;
	for (const auto Index : Indices)
	{
		if (ItemList.IsValidIndex(Index))
		{
			RemovedIndices.Add(Index);
		}
	}

	OnItemListRemove.Broadcast(RemovedIndices);

	for (const auto Index : RemovedIndices)
	{
		ItemList.RemoveAt(Index, EAllowShrinking::No);
	}
	ItemList.Shrink();
	
	MarkArrayDirty();
}

void FInventoryItemList::MarkIndexDirty(TArray<int32> Indices)
{
	TArray<int32> ChangedIndices;
	for (const auto Index : Indices)
	{
		if (ItemList.IsValidIndex(Index))
		{
			ChangedIndices.Add(Index);
			MarkItemDirty(ItemList[Index]);
		}
	}

	OnItemListChange.Broadcast(ChangedIndices);
}

void FInventoryItemList::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	TArray<int> Indices;
	Indices.Append(RemovedIndices);
	OnItemListRemove.Broadcast(Indices);
}

void FInventoryItemList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	TArray<int> Indices;
	Indices.Append(AddedIndices);
	OnItemListAdd.Broadcast(Indices);
}

void FInventoryItemList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	TArray<int> Indices;
	Indices.Append(ChangedIndices);
	OnItemListChange.Broadcast(Indices);
}

bool FInventoryItemList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryItemEntry, FInventoryItemList>(ItemList, DeltaParams, *this);
}