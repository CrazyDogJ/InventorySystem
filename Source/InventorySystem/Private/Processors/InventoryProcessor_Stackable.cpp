// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/InventoryProcessor_Stackable.h"

#include "InventoryItemActor.h"
#include "InventoryItemDefinition.h"
#include "InventoryItemInstance.h"
#include "InventorySystemLibrary.h"
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
	if (ItemInstance)
		return ItemInstance->GetClass()->ImplementsInterface(UStackableItem::StaticClass());
	return false;
}

int UItemProcessor_Stackable::GetEntryStackAmount(const FInventoryItemEntry& ItemEntry)
{
	// Interface
	if (IsStackableItem(ItemEntry.ItemInstance))
	{
		return IStackableItem::Execute_GetStackAmount(ItemEntry.ItemInstance);
	}
	// Default
	return ItemEntry.ItemStack;
}

int UItemProcessor_Stackable::GetEntryMaxStackAmount(const FInventoryItemEntry& ItemEntry)
{
	// Interface
	if (IsStackableItem(ItemEntry.ItemInstance))
	{
		return IStackableItem::Execute_GetMaxStackAmount(ItemEntry.ItemInstance);
	}
	// Default
	return GetDefaultMaxStackAmount(ItemEntry.ItemDefinition);
}

bool UItemProcessor_Stackable::CanEntryStack(const FInventoryItemEntry& InItemEntry,
                                             const FInventoryItemEntry& OtherItemEntry)
{
	// Validate
	if (InItemEntry.IsSlotEmpty() || OtherItemEntry.IsSlotEmpty())
	{
		return false;
	}
	if (InItemEntry.ItemDefinition != OtherItemEntry.ItemDefinition)
	{
		return false;
	}
	// Interface
	if (IsStackableItem(InItemEntry.ItemInstance))
	{
		return IStackableItem::Execute_CanStack(InItemEntry.ItemInstance, OtherItemEntry.ItemInstance);
	}
	// Default
	const auto DefaultMaxStack = GetDefaultMaxStackAmount(InItemEntry.ItemDefinition);
	const auto ToMax = DefaultMaxStack - InItemEntry.ItemStack;
	return ToMax >= 1;
}

int UItemProcessor_Stackable::FindFirstStackableSlot(const FInventoryItemList& ItemList, const FInventoryItemEntry& ItemEntry)
{
	for (int i = 0; i < ItemList.ItemList.Num(); ++i)
	{
		const auto ItrEntry = ItemList.ItemList[i];
		if (CanEntryStack(ItrEntry, ItemEntry))
		{
			return i;
		}
	}
	
	return INDEX_NONE;
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
	if (IsStackableItem(ItemEntry.ItemInstance))
	{
		return IStackableItem::Execute_StackItemInstance(ItemEntry.ItemInstance, InItemEntry.ItemInstance, Amount);
	}

	const auto MaxStackAmount = GetDefaultMaxStackAmount(ItemEntry.ItemDefinition);
	const auto InItemStackAmount = InItemEntry.ItemStack;
	const auto CachedStackAmount = ItemEntry.ItemStack;
	// Custom stack in amount.
	if (Amount > 0)
	{
		ItemEntry.ItemStack = FMath::Clamp(ItemEntry.ItemStack + Amount, 1, MaxStackAmount);
	}
	else if (Amount < 0)
	{
		ItemEntry.ItemStack = FMath::Clamp(ItemEntry.ItemStack + InItemStackAmount, 1, MaxStackAmount);
	}
	const auto Delta = ItemEntry.ItemStack - CachedStackAmount;
	if (InItemEntry.ItemStack - Delta <= 0)
	{
		InItemEntry.EmptySlot();
		return true;
	}

	InItemEntry.ItemStack -= Delta;
	return false;
}

bool UItemProcessor_Stackable::AddItem(FInventoryItemList& ItemList, FInventoryItemEntry& ItemEntry)
{
	TArray<int32> DirtyIndices;
	if (ItemEntry.IsSlotEmpty()) return false;

	// 1. Stack
	int StackIndex = FindFirstStackableSlot(ItemList, ItemEntry);
	while (StackIndex >= 0)
	{
		const auto StackResult = StackItemEntry(ItemList.ItemList[StackIndex], ItemEntry);
		ItemList.MarkIndexDirty({StackIndex});
		if (StackResult)
		{
			return true;
		}

		// Find stack again.
		StackIndex = FindFirstStackableSlot(ItemList, ItemEntry);
	}

	// 2. Empty
	int EmptyIndex = UInventorySystemLibrary::FindFirstEmptySlot(ItemList);
	while (EmptyIndex >= 0)
	{
		const auto AmountToMax = GetEntryMaxStackAmount(ItemEntry);

		// End
		if (AmountToMax >= GetEntryStackAmount(ItemEntry))
		{
			ItemList.ItemList[EmptyIndex] = ItemEntry;
			if (ItemEntry.ItemInstance)
			{
				ItemEntry.ItemInstance->ChangeOuter(ItemList.OuterObject);
			}
			DirtyIndices.Add(EmptyIndex);
			ItemList.MarkIndexDirty(DirtyIndices);
			ItemEntry = FInventoryItemEntry();
			return true;
		}

		// Loop
		ItemList.ItemList[EmptyIndex].ItemDefinition = ItemEntry.ItemDefinition;
		if (const auto NewInstance = UInventoryItemInstance::NewItemInstance(ItemList.OuterObject, ItemEntry.ItemDefinition))
		{
			ItemList.ItemList[EmptyIndex].ItemInstance = NewInstance;
			if (IsStackableItem(NewInstance))
			{
				IStackableItem::Execute_StackItemInstance(NewInstance, ItemEntry.ItemInstance, -1);
			}
			else
			{
				ItemList.ItemList[EmptyIndex].ItemStack = AmountToMax;
				ItemEntry.ItemStack -= AmountToMax;
			}
		}
		else
		{
			ItemList.ItemList[EmptyIndex].ItemStack = AmountToMax;
			ItemEntry.ItemStack -= AmountToMax;
		}
		
		// Instance
		DirtyIndices.Add(EmptyIndex);
		
		// Find empty again.
		EmptyIndex = UInventorySystemLibrary::FindFirstEmptySlot(ItemList);
	}

	ItemList.MarkIndexDirty(DirtyIndices);
	return false;
}

bool UItemProcessor_Stackable::PickupItemActor(FInventoryItemList& ItemList, AInventoryItemActor* ItemActor)
{
	if (ItemActor != nullptr)
	{
		if (AddItem(ItemList, ItemActor->ItemEntry))
		{
			ItemActor->NotifyItemActorPickedUp();
		}
		else
		{
			return false;
		}
	}
	
	return false;
}

int UItemProcessor_Stackable::GetItemTotalAmountByDefinition(const FInventoryItemList& ItemList,
	UInventoryItemDefinition* ItemDef)
{
	int Result = 0;
	
	for (auto Itr : ItemList.ItemList)
	{
		if (!Itr.IsSlotEmpty() && Itr.ItemInstance->ItemDefinition == ItemDef)
		{
			Result += GetEntryStackAmount(Itr);
		}
	}
	
	return Result;
}

int UItemProcessor_Stackable::RemoveItemByDefinition(FInventoryItemList& ItemList, UInventoryItemDefinition* ItemDef,
                                                      int Amount, bool bForceRemove)
{
	int LocalAmount = Amount;
	// Valid check.
	if (!bForceRemove)
	{
		if (GetItemTotalAmountByDefinition(ItemList, ItemDef) < Amount)
		{
			return Amount;
		}
	}

	TArray<int> EmptyIndex;
	TArray<int> DirtyIndex;
	for (int i = ItemList.ItemList.Num() - 1; i >= 0; --i)
	{
		if (!ItemList.ItemList[i].IsSlotEmpty() && ItemList.ItemList[i].ItemDefinition == ItemDef && LocalAmount > 0)
		{
			if (GetEntryStackAmount(ItemList.ItemList[i]) > LocalAmount)
			{
				if (IsStackableItem(ItemList.ItemList[i].ItemInstance))
				{
					IStackableItem::Execute_RemoveStack(ItemList.ItemList[i].ItemInstance, LocalAmount);
				}
				else
				{
					ItemList.ItemList[i].ItemStack -= LocalAmount;
				}
				break;
			}
			
			if (GetEntryStackAmount(ItemList.ItemList[i]) == LocalAmount)
			{
				EmptyIndex.Add(i);
				DirtyIndex.Add(i);
				break;
			}
			
			EmptyIndex.Add(i);
			DirtyIndex.Add(i);
			LocalAmount -= GetEntryStackAmount(ItemList.ItemList[i]);
		}
	}

	ItemList.EmptySlotByIndex(EmptyIndex);
	ItemList.MarkIndexDirty(DirtyIndex);
	return LocalAmount;
}

bool UItemProcessor_Stackable::DragDropItem(FInventoryItemList& DragItemList, int DragIndex,
	FInventoryItemList& DropItemList, int DropIndex)
{
	// Not valid when drag slot is empty!!!
	if (UInventorySystemLibrary::IsSlotEmpty(DragItemList, DragIndex) ||
		!DropItemList.ItemList.IsValidIndex(DropIndex) ||
		!DragItemList.ItemList.IsValidIndex(DragIndex)) return false;

	const auto DragItemInstance = DragItemList.ItemList[DragIndex].ItemInstance;
	const auto DropItemInstance = DropItemList.ItemList[DropIndex].ItemInstance;
	
	const auto DragItemDef = DragItemList.ItemList[DragIndex].ItemDefinition;
	const auto DropItemDef = DropItemList.ItemList[DropIndex].ItemDefinition;
	
	const auto DragItemStackAmount = GetEntryStackAmount(DragItemList.ItemList[DragIndex]);
	const auto DropItemStackAmount = GetEntryStackAmount(DropItemList.ItemList[DropIndex]);

	const auto ItemMaxStackAmount = GetEntryMaxStackAmount(DragItemList.ItemList[DragIndex]);
	const bool bIsSame = DragItemDef == DropItemDef;
	const bool bSameExchange = bIsSame && (ItemMaxStackAmount == DragItemStackAmount || ItemMaxStackAmount == DropItemStackAmount);
	
	// 1. Not the same definition or Max stack exchange
	if (!bIsSame || bSameExchange)
	{
		DragItemList.ItemList[DragIndex].ItemDefinition = DropItemDef;
		DragItemList.ItemList[DragIndex].ItemStack = DropItemStackAmount;
		DragItemList.ItemList[DragIndex].ItemInstance = DropItemInstance;
		DropItemList.ItemList[DropIndex].ItemDefinition = DragItemDef;
		DropItemList.ItemList[DropIndex].ItemStack = DragItemStackAmount;
		DropItemList.ItemList[DropIndex].ItemInstance = DragItemInstance;

		if (DropItemInstance)
		{
			DropItemInstance->ChangeOuter(DragItemList.OuterObject);
		}
		if (DragItemInstance)
		{
			DragItemInstance->ChangeOuter(DropItemList.OuterObject);
		}
		
		DragItemList.MarkIndexDirty({DragIndex});
		DropItemList.MarkIndexDirty({DropIndex});
		return true;
	}

	// 2. Is the same definition
	StackItemEntry(DropItemList.ItemList[DropIndex], DragItemList.ItemList[DragIndex]);
	DragItemList.MarkIndexDirty({DragIndex});
	DropItemList.MarkIndexDirty({DropIndex});
	return true;
}

bool UItemProcessor_Stackable::QuickMoveItem(FInventoryItemList& FromItemList, int FromIndex,
	FInventoryItemList& ToItemList)
{
	if (FromItemList.ItemList.IsValidIndex(FromIndex))
	{
		if (AddItem(ToItemList, FromItemList.ItemList[FromIndex]))
		{
			FromItemList.MarkIndexDirty({FromIndex});
			return true;
		}
	}

	return false;
}

bool UItemProcessor_Stackable::CanSlotSplit(FInventoryItemList& ItemList, int SlotIndex)
{
	if (ItemList.ItemList.IsValidIndex(SlotIndex))
	{
		return GetEntryStackAmount(ItemList.ItemList[SlotIndex]) > 1;
	}
	
	return false;
}

bool UItemProcessor_Stackable::SplitItem(FInventoryItemList& SplitItemList, int SplitIndex, int SplitAmount,
	FInventoryItemList& ContainerToSplit)
{
	if (!SplitItemList.ItemList.IsValidIndex(SplitIndex))
	{
		return false;
	}
	if (SplitItemList.ItemList[SplitIndex].IsSlotEmpty())
	{
		return false;
	}
	if (!CanSlotSplit(SplitItemList, SplitIndex) || SplitAmount < 1)
	{
		return false;
	}

	const auto FoundEmptyIndex = UInventorySystemLibrary::FindFirstEmptySlot(ContainerToSplit);
	if (FoundEmptyIndex >= 0)
	{
		ContainerToSplit.ItemList[FoundEmptyIndex].ItemDefinition = SplitItemList.ItemList[SplitIndex].ItemDefinition;
		if (const auto NewInstance = UInventoryItemInstance::NewItemInstance(ContainerToSplit.OuterObject, SplitItemList.ItemList[SplitIndex].ItemDefinition))
		{
			ContainerToSplit.ItemList[FoundEmptyIndex].ItemInstance = NewInstance;
			if (IsStackableItem(NewInstance))
			{
				IStackableItem::Execute_StackItemInstance(NewInstance, SplitItemList.ItemList[SplitIndex].ItemInstance, -1);
			}
			else
			{
				ContainerToSplit.ItemList[FoundEmptyIndex].ItemStack = SplitAmount;
				SplitItemList.ItemList[SplitIndex].ItemStack -= SplitAmount;
			}
		}
		else
		{
			ContainerToSplit.ItemList[FoundEmptyIndex].ItemStack = SplitAmount;
			SplitItemList.ItemList[SplitIndex].ItemStack -= SplitAmount;
		}
			
		SplitItemList.MarkIndexDirty({SplitIndex});
		ContainerToSplit.MarkIndexDirty({FoundEmptyIndex});
		return true;
	}
	
	return false;
}

/**
void UItemProcessor_Stackable::TidyUpInventory(FInventoryItemList& FromItemList)
{
}

AInventoryItemActor* UItemProcessor_Stackable::DropItem(FInventoryItemList& FromItemList, FTransform ItemActorTransform, int Index, int Amount)
{
}
*/
