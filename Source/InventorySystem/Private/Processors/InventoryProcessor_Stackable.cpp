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

bool UItemInstance_Stackable::StackItemInstance_Implementation(UInventoryItemInstance*& ItemInstance)
{
	if (!Execute_CanStack(this, ItemInstance))
	{
		return false;
	}
	
	const auto MaxStackAmount = Execute_GetMaxStackAmount(this);
	const auto InItemStackAmount = UItemProcessor_Stackable::IsStackableItem(ItemInstance) ? Execute_GetStackAmount(ItemInstance) : 1;
	const auto CachedStackAmount = StackAmount;
	StackAmount = FMath::Clamp(StackAmount + InItemStackAmount, 1, MaxStackAmount);
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

int UItemProcessor_Stackable::FindFirstStackableSlot(const FInventoryItemList& ItemList, UInventoryItemInstance* ItemInstance)
{
	for (int i = 0; i < ItemList.ItemList.Num(); ++i)
	{
		const auto Instance = ItemList.ItemList[i].ItemInstance;
		if (!Instance) continue;

		if (IsStackableItem(Instance))
		{
			if (IStackableItem::Execute_CanStack(Instance, ItemInstance))
			{
				return i;
			}
		}
	}
	
	return INDEX_NONE;
}

int UItemProcessor_Stackable::GetDefaultMaxStackAmount(UInventoryItemDefinition* ItemDef)
{
	if (!ItemDef) return INDEX_NONE;

	// Will get if exist.
	if (const auto StackablePtr = ItemDef->GetFragmentPtr<FItemFragment_Stackable>())
	{
		return StackablePtr->MaxStackAmount;
	}

	// Will be one if not exist.
	return 1;
}

bool UItemProcessor_Stackable::AddItem(FInventoryItemList& ItemList, UInventoryItemInstance*& InItemInstance)
{
	TArray<int32> DirtyIndices;
	if (!InItemInstance) return false;
	const auto InStackableItemAmount = IsStackableItem(InItemInstance) ?
		IStackableItem::Execute_GetStackAmount(InItemInstance) : 1;

	// 1. Stack
	int StackIndex = FindFirstStackableSlot(ItemList, InItemInstance);
	while (StackIndex >= 0)
	{
		UInventoryItemInstance* StackableItemInstance = ItemList.ItemList[StackIndex].ItemInstance;
		if (IStackableItem::Execute_StackItemInstance(StackableItemInstance, InItemInstance))
		{
			return true;
		}

		// Find stack again.
		StackIndex = FindFirstStackableSlot(ItemList, InItemInstance);
	}

	// 2. Empty
	int EmptyIndex = UInventorySystemLibrary::FindFirstEmptySlot(ItemList);
	while (EmptyIndex >= 0)
	{
		const auto AmountToMax = IsStackableItem(InItemInstance) ?
		IStackableItem::Execute_GetMaxStackAmount(InItemInstance) : 1;

		// End
		if (AmountToMax >= InStackableItemAmount)
		{
			ItemList.ItemList[EmptyIndex].ItemInstance = InItemInstance;
			InItemInstance->ChangeOuter(ItemList.OuterObject);
			DirtyIndices.Add(EmptyIndex);
			ItemList.MarkIndexDirty(DirtyIndices);
			InItemInstance = nullptr;
			return true;
		}

		// Loop
		ItemList.ItemList[EmptyIndex].ItemInstance = UInventoryItemInstance::NewItemInstance(ItemList.OuterObject, InItemInstance->ItemDefinition);
		if (IsStackableItem(ItemList.ItemList[EmptyIndex].ItemInstance))
		{
			IStackableItem::Execute_StackItemInstance(ItemList.ItemList[EmptyIndex].ItemInstance, InItemInstance);
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
		if (AddItem(ItemList, ItemActor->ItemInstance))
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
			Result += IsStackableItem(Itr.ItemInstance) ?
				IStackableItem::Execute_GetStackAmount(Itr.ItemInstance) : 1;
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
		const auto Instance = ItemList.ItemList[i].ItemInstance;
		if (!ItemList.ItemList[i].IsSlotEmpty() && Instance->ItemDefinition == ItemDef && LocalAmount > 0)
		{
			const auto InstanceStackAmount = IsStackableItem(Instance) ?
				IStackableItem::Execute_GetStackAmount(Instance) : 1;
			
			if (InstanceStackAmount > LocalAmount)
			{
				if (IsStackableItem(Instance))
				{
					IStackableItem::Execute_RemoveStack(Instance, LocalAmount);
				}
				break;
			}
			
			if (InstanceStackAmount == LocalAmount)
			{
				EmptyIndex.Add(i);
				DirtyIndex.Add(i);
				break;
			}
			
			EmptyIndex.Add(i);
			DirtyIndex.Add(i);
			LocalAmount -= InstanceStackAmount;
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
	if (UInventorySystemLibrary::IsSlotEmpty(DragItemList, DragIndex) || !DropItemList.ItemList.IsValidIndex(DropIndex)) return false;

	auto DragItemInstance = DragItemList.ItemList[DragIndex].ItemInstance;
	auto DropItemInstance = DropItemList.ItemList[DropIndex].ItemInstance;
	
	const auto DragItemDef = DragItemInstance ? DragItemInstance->ItemDefinition : nullptr;
	const auto DropItemDef = DropItemInstance ? DropItemInstance->ItemDefinition : nullptr;
	
	const auto DragItemStackAmount = IsStackableItem(DragItemInstance) ?
		IStackableItem::Execute_GetStackAmount(DragItemInstance) : 1;
	const auto DropItemStackAmount = IsStackableItem(DropItemInstance) ?
		IStackableItem::Execute_GetStackAmount(DropItemInstance) : 1;

	const auto ItemMaxStackAmount = IsStackableItem(DragItemInstance) ?
		IStackableItem::Execute_GetMaxStackAmount(DragItemInstance) : 1;
	const bool bIsSame = DragItemDef == DropItemDef;
	const bool bSameExchange = bIsSame && (ItemMaxStackAmount == DragItemStackAmount || ItemMaxStackAmount == DropItemStackAmount);
	
	// 1. Not the same definition or Max stack exchange
	if (!bIsSame || bSameExchange)
	{
		const auto TempDragInstance = DragItemList.ItemList[DragIndex].ItemInstance;
		const auto TempDropInstance = DropItemList.ItemList[DropIndex].ItemInstance;

		DragItemList.ItemList[DragIndex].ItemInstance = TempDropInstance;
		DropItemList.ItemList[DropIndex].ItemInstance = TempDragInstance;

		if (TempDropInstance)
		{
			TempDropInstance->ChangeOuter(DragItemList.OuterObject);
		}
		TempDragInstance->ChangeOuter(DropItemList.OuterObject);
		
		DragItemList.MarkIndexDirty({DragIndex});
		DropItemList.MarkIndexDirty({DropIndex});
		return true;
	}

	// 2. Is the same definition
	if (IsStackableItem(DropItemInstance))
	{
		if (IStackableItem::Execute_StackItemInstance(DropItemInstance, DragItemInstance))
		{
			DragItemList.MarkIndexDirty({DragIndex});
			return true;
		}

		// Can stack part
		return true;
	}
	
	return false;
}

bool UItemProcessor_Stackable::QuickMoveItem(FInventoryItemList& FromItemList, int FromIndex,
	FInventoryItemList& ToItemList)
{
	if (FromItemList.ItemList.IsValidIndex(FromIndex))
	{
		if (AddItem(ToItemList, FromItemList.ItemList[FromIndex].ItemInstance))
		{
			FromItemList.MarkIndexDirty({FromIndex});
			return true;
		}
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