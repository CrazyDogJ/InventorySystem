// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/InventoryProcessor_Stackable.h"

#include "InventoryContainerComponent.h"
#include "InventoryItemActor.h"
#include "InventoryItemDefinition.h"

int UItemProcessor_Stackable::GetEntryStackAmount(const FInventoryItemEntry& ItemEntry)
{
	return ItemEntry.GetStackAmount();
}

void UItemProcessor_Stackable::SetEntryStackAmount(FInventoryItemEntry& ItemEntry, const int Amount)
{
	ItemEntry.SetStackAmount(Amount);
}

int UItemProcessor_Stackable::GetEntryMaxStackAmount(const FInventoryItemEntry& ItemEntry)
{
	return ItemEntry.GetMaxStackAmount();
}

bool UItemProcessor_Stackable::CanEntryStack(const FInventoryItemEntry& InItemEntry,
                                             const FInventoryItemEntry& OtherItemEntry)
{
	return InItemEntry.CanEntryStack(OtherItemEntry);
}

int UItemProcessor_Stackable::FindFirstStackableSlot(const FInventoryItemList& ItemList, const FInventoryItemEntry& ItemEntry)
{
	return ItemList.FindFirstStackableSlot(ItemEntry);
}

int UItemProcessor_Stackable::GetDefaultMaxStackAmount(const UInventoryItemDefinition* ItemDef)
{
	if (!ItemDef) return INDEX_NONE;

	// Will get if existed.
	if (const auto StackablePtr = ItemDef->GetFragmentPtr<FItemFragment_Stackable>())
	{
		return StackablePtr->MaxStackAmount;
	}

	// Will be one if not exist.
	return 1;
}

bool UItemProcessor_Stackable::StackItemEntry(FInventoryItemEntry& ItemEntry, FInventoryItemEntry& InItemEntry, int Amount)
{
	return ItemEntry.StackEntry(InItemEntry, Amount);
}

bool UItemProcessor_Stackable::AddItem(FInventoryItemList& ItemList, FInventoryItemEntry& ItemEntry)
{
	return ItemList.AddItem(ItemEntry);
}

bool UItemProcessor_Stackable::PickupItemObject(FInventoryItemList& ItemList, UObject* ItemObject, const int InstanceIndex)
{
	if (ItemObject != nullptr)
	{
		if (ItemObject->Implements<UInventoryEntryInterface>())
		{
			if (IInventoryEntryInterface::Execute_IsItemEntryValid(ItemObject, InstanceIndex))
			{
				FInventoryItemEntry Entry = IInventoryEntryInterface::Execute_GetItemEntry(ItemObject);
				const auto CachedItemDef = Entry.ItemDefinition;
				const auto CachedAmount = Entry.GetStackAmount();
				const auto AddItemBool = ItemList.AddItem(Entry);
				IInventoryEntryInterface::Execute_SetItemEntry(ItemObject, Entry);
				if (AddItemBool)
				{
					IInventoryEntryInterface::Execute_OnEntryPickedUp(ItemObject, InstanceIndex);
				}
				
				// Trigger visual multicast.
				if (const auto Container = Cast<UInventoryContainerComponent>(ItemList.OuterObject))
				{
					Container->OnItemPickupUpClient(CachedItemDef, CachedAmount - Entry.GetStackAmount());
				}
			}
		}
	}
	
	return false;
}

int UItemProcessor_Stackable::GetItemTotalAmountByDefinition(const FInventoryItemList& ItemList,
                                                             UInventoryItemDefinition* ItemDef)
{
	return ItemList.GetItemTotalAmountByDefinition(ItemDef);
}

bool UItemProcessor_Stackable::RemoveStackAtIndex(FInventoryItemList& ItemList, const int Index, const int Amount, const bool bForceRemove)
{
	return ItemList.RemoveStackAtIndex(Index, Amount, bForceRemove);
}

bool UItemProcessor_Stackable::IsItemStackMappingEqual(const FItemStackMapping& AMap, const FItemStackMapping& BMap)
{
	return AMap == BMap;
}

FItemStackMapping UItemProcessor_Stackable::GetItemStackMapping(const FInventoryItemList& InItemList)
{
	return InItemList.GetItemMappings();
}

bool UItemProcessor_Stackable::CanListRemoveItems(const FInventoryItemList& InItemList,
	const FItemStackMapping& InItems)
{
	return InItemList.CanRemoveItems(InItems);
}

bool UItemProcessor_Stackable::CanListAddItems(const FInventoryItemList& InItemList, const FItemStackMapping& InItems)
{
	return InItemList.CanAddItems(InItems);
}

bool UItemProcessor_Stackable::CanListExchangeItems(const FInventoryItemList& InItemList,
	const FItemStackMapping& RemoveItems, const FItemStackMapping& AddItems)
{
	return InItemList.CanExchangeItems(RemoveItems, AddItems);
}

bool UItemProcessor_Stackable::ExchangeItems(FInventoryItemList& InItemList, const FItemStackMapping& RemoveItems,
	const FItemStackMapping& AddItems)
{
	return InItemList.ExchangeItems(RemoveItems, AddItems);
}

bool UItemProcessor_Stackable::AddItemByDefinition(FInventoryItemList& ItemList, UInventoryItemDefinition* ItemDef,
                                                   const int Amount)
{
	return ItemList.AddItemByDefinition(ItemDef, Amount);
}

int UItemProcessor_Stackable::RemoveItemByDefinition(FInventoryItemList& ItemList, UInventoryItemDefinition* ItemDef,
                                                     int Amount, bool bForceRemove)
{
	return ItemList.RemoveItemByDefinition(ItemDef, Amount, bForceRemove);
}

bool UItemProcessor_Stackable::DragDropItem(FInventoryItemList& DragItemList, int DragIndex,
	FInventoryItemList& DropItemList, int DropIndex)
{
	return DragItemList.DragDropItem(DragIndex, DropItemList, DropIndex);
}

bool UItemProcessor_Stackable::QuickMoveItem(FInventoryItemList& FromItemList, int FromIndex,
	FInventoryItemList& ToItemList)
{
	return FromItemList.QuickMoveItem(FromIndex, ToItemList);
}

bool UItemProcessor_Stackable::CanSlotSplit(FInventoryItemList& ItemList, int SlotIndex)
{
	return ItemList.CanSlotSplit(SlotIndex);
}

bool UItemProcessor_Stackable::SplitItem(FInventoryItemList& SplitItemList, int SplitIndex, int SplitAmount,
	FInventoryItemList& ContainerToSplit)
{
	return SplitItemList.SplitItem(SplitIndex, SplitAmount, ContainerToSplit);
}

/**
void UItemProcessor_Stackable::TidyUpInventory(FInventoryItemList& FromItemList)
{
}

AInventoryItemActor* UItemProcessor_Stackable::DropItem(FInventoryItemList& FromItemList, FTransform ItemActorTransform, int Index, int Amount)
{
}
*/
