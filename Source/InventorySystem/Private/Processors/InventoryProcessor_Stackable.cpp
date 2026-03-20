// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/InventoryProcessor_Stackable.h"

#include "InventoryContainerComponent.h"
#include "InventoryItemActor.h"
#include "InventoryItemDefinition.h"
#include "InventoryItemInstance.h"
#include "Net/UnrealNetwork.h"

#if WITH_EDITOR
void UItemInstance_Stackable::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, StackAmount))
	{
		StackAmount = FMath::Clamp(StackAmount, 1, Execute_GetMaxStackAmount(this));
	}
}
#endif

bool UItemInstance_Stackable::CanStack_Implementation(const UInventoryItemInstance* ItemInstance) const
{
	if (ItemDefinition && ItemInstance && ItemInstance->ItemDefinition)
	{
		if (ItemDefinition == ItemInstance->ItemDefinition)
		{
			const auto MaxStackAmount = Execute_GetMaxStackAmount(this);
			return StackAmount < MaxStackAmount;
		}
	}
	
	return IStackableItem::CanStack_Implementation(ItemInstance);
}

bool UItemInstance_Stackable::StackItemInstance_Implementation(UInventoryItemInstance*& ItemInstance, int Amount)
{
	if (Amount == 0)
	{
		return false;
	}
	
	if (ItemInstance == nullptr)
	{
		return false;
	}
	
	if (!Execute_CanStack(this, ItemInstance))
	{
		return false;
	}
	
	const auto MaxStackAmount = Execute_GetMaxStackAmount(this);
	const auto InItemStackAmount = UItemProcessor_Stackable::IsStackableItem(ItemInstance) ? Execute_GetStackAmount(ItemInstance) : 1;
	const auto CachedStackAmount = StackAmount;
	// Custom stack in amount.
	if (Amount > 0)
	{
		StackAmount = FMath::Clamp(StackAmount + Amount, 1, MaxStackAmount);
	}
	else if (Amount < 0)
	{
		StackAmount = FMath::Clamp(StackAmount + InItemStackAmount, 1, MaxStackAmount);
	}
	const auto Delta = StackAmount - CachedStackAmount;
	if (UItemProcessor_Stackable::IsStackableItem(ItemInstance))
	{
		if (Execute_RemoveStack(ItemInstance, Delta) <= 0)
		{
			ItemInstance = nullptr;
			return true;
		}
	}
	
	return IStackableItem::StackItemInstance_Implementation(ItemInstance);
}

int UItemInstance_Stackable::RemoveStack_Implementation(int InAmount)
{
	StackAmount -= InAmount;
	if (StackAmount <= 0)
	{
		ConditionalBeginDestroy();
		return StackAmount;
	}

	return StackAmount;
}

int UItemInstance_Stackable::GetMaxStackAmount_Implementation() const
{
	if (ItemDefinition)
	{
		return UItemProcessor_Stackable::GetDefaultMaxStackAmount(ItemDefinition);
	}
	
	return IStackableItem::GetMaxStackAmount_Implementation();
}

void UItemInstance_Stackable::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StackAmount);
}

bool UItemProcessor_Stackable::IsStackableItem(const UInventoryItemInstance* ItemInstance)
{
	if (ItemInstance && ItemInstance->Implements<UStackableItem>())
	{
		return true;
	}
	
	return false;
}

int UItemProcessor_Stackable::GetEntryStackAmount(const FInventoryItemEntry& ItemEntry)
{
	return ItemEntry.GetStackAmount();
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

int UItemProcessor_Stackable::GetDefaultMaxStackAmount(UInventoryItemDefinition* ItemDef)
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

bool UItemProcessor_Stackable::RemoveStackAtIndex(FInventoryItemList& ItemList, int Index, int Amount, bool bForceRemove)
{
	return ItemList.RemoveStackAtIndex(Index, Amount,bForceRemove);
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
