// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryListContainer.h"
#include "InventoryItemDefinition.h"
#include "Processors/InventoryProcessor_Stackable.h"

FInventoryItemEntry::FInventoryItemEntry(UObject* OuterObject, UInventoryItemDefinition* InItemDefinition)
{
	ItemDefinition = InItemDefinition;
	if (ItemDefinition)
	{
		ItemInstance = UInventoryItemInstance::NewItemInstance(OuterObject, ItemDefinition);
		if (!ItemInstance)
		{
			ItemStack = 1;
		}
	}
}

bool FInventoryItemEntry::IsSlotEmpty() const
{
	return ItemDefinition == nullptr && ItemStack == 0 && ItemInstance == nullptr;
}

void FInventoryItemEntry::EmptySlot(const bool& bDestroyInstance)
{
	ItemDefinition = nullptr;
	ItemStack = 0;
	if (ItemInstance && bDestroyInstance)
	{
		ItemInstance->ConditionalBeginDestroy();
	}
	ItemInstance = nullptr;
}

int FInventoryItemEntry::GetStackAmount() const
{
	// Interface
	if (UItemProcessor_Stackable::IsStackableItem(ItemInstance))
	{
		return IStackableItem::Execute_GetStackAmount(ItemInstance);
	}
	// Default
	return ItemStack;
}

int FInventoryItemEntry::GetMaxStackAmount() const
{
	// Interface
	if (UItemProcessor_Stackable::IsStackableItem(ItemInstance))
	{
		return IStackableItem::Execute_GetMaxStackAmount(ItemInstance);
	}
	// Default
	return UItemProcessor_Stackable::GetDefaultMaxStackAmount(ItemDefinition);
}

bool FInventoryItemEntry::CanEntryStack(const FInventoryItemEntry& OtherItemEntry) const
{
	// Validate
	if (IsSlotEmpty() || OtherItemEntry.IsSlotEmpty())
	{
		return false;
	}
	if (ItemDefinition != OtherItemEntry.ItemDefinition)
	{
		return false;
	}
	// Interface
	if (UItemProcessor_Stackable::IsStackableItem(ItemInstance))
	{
		return IStackableItem::Execute_CanStack(ItemInstance, OtherItemEntry.ItemInstance);
	}
	// Default
	const auto DefaultMaxStack = UItemProcessor_Stackable::GetDefaultMaxStackAmount(ItemDefinition);
	const auto ToMax = DefaultMaxStack - ItemStack;
	return ToMax >= 1;
}

bool FInventoryItemEntry::StackEntry(FInventoryItemEntry& OtherEntry, int SpecificAmount)
{
	if (UItemProcessor_Stackable::IsStackableItem(ItemInstance))
	{
		return IStackableItem::Execute_StackItemInstance(ItemInstance, OtherEntry.ItemInstance, SpecificAmount);
	}

	const auto MaxStackAmount = UItemProcessor_Stackable::GetDefaultMaxStackAmount(ItemDefinition);
	const auto InItemStackAmount = OtherEntry.ItemStack;
	const auto CachedStackAmount = ItemStack;
	// Custom stack in amount.
	if (SpecificAmount > 0)
	{
		ItemStack = FMath::Clamp(ItemStack + SpecificAmount, 1, MaxStackAmount);
	}
	else if (SpecificAmount == INDEX_NONE)
	{
		ItemStack = FMath::Clamp(ItemStack + InItemStackAmount, 1, MaxStackAmount);
	}
	const auto Delta = ItemStack - CachedStackAmount;
	if (OtherEntry.ItemStack - Delta <= 0)
	{
		OtherEntry.EmptySlot();
		return true;
	}

	OtherEntry.ItemStack -= Delta;
	return false;
}

void FInventoryItemEntry::RemoveStack(int SpecificAmount)
{
	if (IsSlotEmpty() || SpecificAmount <= 0)
	{
		return;
	}
	
	if (UItemProcessor_Stackable::IsStackableItem(ItemInstance))
	{
		IStackableItem::Execute_RemoveStack(ItemInstance, SpecificAmount);
	}

	ItemStack -= SpecificAmount;
	if (ItemStack <= 0)
	{
		EmptySlot();
	}
}

void FInventoryItemList::EmptySlotByIndex(TArray<int32> Indices, bool bDirty)
{
	for (const int Index : Indices)
	{
		if (ItemList.IsValidIndex(Index))
		{
			ItemList[Index].EmptySlot();
			if (bDirty)
			{
				MarkIndexDirty(Index);
			}
		}
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

void FInventoryItemList::MarkIndexDirty(int32 Index)
{
	if (ItemList.IsValidIndex(Index))
	{
		MarkItemDirty(ItemList[Index]);
		OnItemListChange.Broadcast({Index});
	}
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

bool FInventoryItemList::IsSlotEmpty(int Index, bool& Empty) const
{
	if (ItemList.IsValidIndex(Index))
	{
		Empty = ItemList[Index].IsSlotEmpty();
		return true;
	}
	
	return false;
}

int FInventoryItemList::FindFirstEmptySlot() const
{
	for (int i = 0; i < ItemList.Num(); ++i)
	{
		if (ItemList[i].IsSlotEmpty())
		{
			return i;
		}
	}
	
	return INDEX_NONE;
}

int FInventoryItemList::FindFirstStackableSlot(const FInventoryItemEntry& OtherItemEntry) const
{
	for (int i = 0; i < ItemList.Num(); ++i)
    {
    	const auto ItrEntry = ItemList[i];
    	if (ItrEntry.CanEntryStack(OtherItemEntry))
    	{
    		return i;
    	}
    }
    
    return INDEX_NONE;
}

int FInventoryItemList::GetItemTotalAmountByDefinition(const UInventoryItemDefinition* ItemDef) const
{
	int Result = 0;
	
	for (auto Itr : ItemList)
	{
		if (!Itr.IsSlotEmpty() && Itr.ItemDefinition == ItemDef)
		{
			Result += Itr.GetStackAmount();
		}
	}
	
	return Result;
}

UInventoryItemDefinition* FInventoryItemList::GetItemDefinition(const int& Index) const
{
	if (ItemList.IsValidIndex(Index))
	{
		return ItemList[Index].ItemDefinition;
	}

	return nullptr;
}

UInventoryItemInstance* FInventoryItemList::GetItemInstance(const int& Index) const
{
	if (ItemList.IsValidIndex(Index))
	{
		return ItemList[Index].ItemInstance;
	}

	return nullptr;
}

bool FInventoryItemList::CanSlotSplit(const int& SlotIndex) const
{
	if (ItemList.IsValidIndex(SlotIndex))
	{
		return ItemList[SlotIndex].GetStackAmount() > 1;
	}
	
	return false;
}

bool FInventoryItemList::RemoveStackAtIndex(const int& Index, const int& Amount, const bool& bForceRemove)
{
	if (!ItemList.IsValidIndex(Index)) return false;
	
	// Valid check.
	if (!bForceRemove)
	{
		if (ItemList[Index].GetStackAmount() < Amount)
		{
			return false;
		}
	}

	ItemList[Index].RemoveStack(Amount);
	MarkIndexDirty(Index);
	return true;
}

int FInventoryItemList::RemoveItemByDefinition(const UInventoryItemDefinition* ItemDef, const int& Amount,
                                               const bool& bForceRemove)
{
	int LocalAmount = Amount;
	// Valid check.
	if (!bForceRemove)
	{
		if (GetItemTotalAmountByDefinition(ItemDef) < Amount)
		{
			return Amount;
		}
	}

	TArray<int> EmptyIndex;
	TArray<int> DirtyIndex;
	for (int i = ItemList.Num() - 1; i >= 0; --i)
	{
		if (!ItemList[i].IsSlotEmpty() && ItemList[i].ItemDefinition == ItemDef && LocalAmount > 0)
		{
			if (ItemList[i].GetStackAmount() > LocalAmount)
			{
				if (UItemProcessor_Stackable::IsStackableItem(ItemList[i].ItemInstance))
				{
					IStackableItem::Execute_RemoveStack(ItemList[i].ItemInstance, LocalAmount);
				}
				else
				{
					ItemList[i].ItemStack -= LocalAmount;
				}
				MarkIndexDirty(i);
				break;
			}
			
			if (ItemList[i].GetStackAmount() == LocalAmount)
			{
				EmptySlotByIndex({i});
				break;
			}
			
			EmptySlotByIndex({i});
			LocalAmount -= ItemList[i].GetStackAmount();
		}
	}
	
	return LocalAmount;
}

bool FInventoryItemList::AddItem(FInventoryItemEntry& ItemEntry)
{
	if (ItemEntry.IsSlotEmpty()) return false;

	// 1. Stack
	int StackIndex = FindFirstStackableSlot(ItemEntry);
	while (StackIndex >= 0)
	{
		const auto StackResult = ItemList[StackIndex].StackEntry(ItemEntry);
		MarkIndexDirty(StackIndex);
		if (StackResult)
		{
			return true;
		}

		// Find stack again.
		StackIndex = FindFirstStackableSlot(ItemEntry);
	}

	// 2. Empty
	int EmptyIndex = FindFirstEmptySlot();
	while (EmptyIndex >= 0)
	{
		const auto AmountToMax = ItemEntry.GetMaxStackAmount();

		// End
		if (AmountToMax >= ItemEntry.GetStackAmount())
		{
			ItemList[EmptyIndex].CopyFrom(ItemEntry);
			if (ItemEntry.ItemInstance)
			{
				ItemEntry.ItemInstance->ChangeOuter(OuterObject);
			}
			MarkIndexDirty(EmptyIndex);
			ItemEntry.EmptySlot(false);
			return true;
		}

		// Loop
		ItemList[EmptyIndex].ItemDefinition = ItemEntry.ItemDefinition;
		if (const auto NewInstance = UInventoryItemInstance::NewItemInstance(OuterObject, ItemEntry.ItemDefinition))
		{
			ItemList[EmptyIndex].ItemInstance = NewInstance;
			if (UItemProcessor_Stackable::IsStackableItem(NewInstance))
			{
				IStackableItem::Execute_StackItemInstance(NewInstance, ItemEntry.ItemInstance, ItemEntry.GetStackAmount());
			}
			else
			{
				ItemList[EmptyIndex].ItemStack = AmountToMax;
				ItemEntry.ItemStack -= AmountToMax;
			}
		}
		else
		{
			ItemList[EmptyIndex].ItemStack = AmountToMax;
			ItemEntry.ItemStack -= AmountToMax;
		}
		
		// Instance
		MarkIndexDirty(EmptyIndex);
		
		// Find empty again.
		EmptyIndex = FindFirstEmptySlot();
	}
	
	return false;
}

bool FInventoryItemList::DragDropItem(int DragIndex, FInventoryItemList& DropItemList, int DropIndex)
{
	// Not valid when drag slot is empty!!!
	
	if (!DropItemList.ItemList.IsValidIndex(DropIndex) ||
		!ItemList.IsValidIndex(DragIndex) ||
		ItemList[DragIndex].IsSlotEmpty()) return false;

	const auto DragItemInstance = ItemList[DragIndex].ItemInstance;
	const auto DropItemInstance = DropItemList.ItemList[DropIndex].ItemInstance;
	
	const auto DragItemDef = ItemList[DragIndex].ItemDefinition;
	const auto DropItemDef = DropItemList.ItemList[DropIndex].ItemDefinition;
	
	const auto DragItemStackAmount = ItemList[DragIndex].GetStackAmount();
	const auto DropItemStackAmount = DropItemList.ItemList[DropIndex].GetStackAmount();

	const auto ItemMaxStackAmount = ItemList[DragIndex].GetMaxStackAmount();
	const bool bIsSame = DragItemDef == DropItemDef;
	const bool bSameExchange = bIsSame && (ItemMaxStackAmount == DragItemStackAmount || ItemMaxStackAmount == DropItemStackAmount);
	
	// 1. Not the same definition or Max stack exchange
	if (!bIsSame || bSameExchange)
	{
		ItemList[DragIndex].ItemDefinition = DropItemDef;
		ItemList[DragIndex].ItemStack = DropItemStackAmount;
		ItemList[DragIndex].ItemInstance = DropItemInstance;
		DropItemList.ItemList[DropIndex].ItemDefinition = DragItemDef;
		DropItemList.ItemList[DropIndex].ItemStack = DragItemStackAmount;
		DropItemList.ItemList[DropIndex].ItemInstance = DragItemInstance;

		if (DropItemInstance)
		{
			DropItemInstance->ChangeOuter(OuterObject);
		}
		if (DragItemInstance)
		{
			DragItemInstance->ChangeOuter(DropItemList.OuterObject);
		}
		
		MarkIndexDirty(DragIndex);
		DropItemList.MarkIndexDirty(DropIndex);
		return true;
	}

	// 2. Is the same definition
	DropItemList.ItemList[DropIndex].StackEntry(ItemList[DragIndex]);
	MarkIndexDirty(DragIndex);
	DropItemList.MarkIndexDirty(DropIndex);
	return true;
}

bool FInventoryItemList::QuickMoveItem(int FromIndex, FInventoryItemList& ToItemList)
{
	if (ItemList.IsValidIndex(FromIndex))
	{
		if (ToItemList.AddItem(ItemList[FromIndex]))
		{
			MarkIndexDirty(FromIndex);
			return true;
		}
	}

	return false;
}

bool FInventoryItemList::SplitItem(int SplitIndex, int SplitAmount, FInventoryItemList& ContainerToSplit)
{
	if (!ItemList.IsValidIndex(SplitIndex))
	{
		return false;
	}
	if (ItemList[SplitIndex].IsSlotEmpty())
	{
		return false;
	}
	if (!CanSlotSplit(SplitIndex) || SplitAmount < 1)
	{
		return false;
	}

	const auto FoundEmptyIndex = FindFirstEmptySlot();
	if (FoundEmptyIndex >= 0)
	{
		ContainerToSplit.ItemList[FoundEmptyIndex].ItemDefinition = ItemList[SplitIndex].ItemDefinition;
		if (const auto NewInstance = UInventoryItemInstance::NewItemInstance(ContainerToSplit.OuterObject, ItemList[SplitIndex].ItemDefinition))
		{
			ContainerToSplit.ItemList[FoundEmptyIndex].ItemInstance = NewInstance;
			if (UItemProcessor_Stackable::IsStackableItem(NewInstance))
			{
				IStackableItem::Execute_StackItemInstance(NewInstance, ItemList[SplitIndex].ItemInstance, SplitAmount);
			}
			else
			{
				ContainerToSplit.ItemList[FoundEmptyIndex].ItemStack = SplitAmount;
				ItemList[SplitIndex].ItemStack -= SplitAmount;
			}
		}
		else
		{
			ContainerToSplit.ItemList[FoundEmptyIndex].ItemStack = SplitAmount;
			ItemList[SplitIndex].ItemStack -= SplitAmount;
		}
			
		MarkIndexDirty(SplitIndex);
		ContainerToSplit.MarkIndexDirty(FoundEmptyIndex);
		return true;
	}
	
	return false;
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