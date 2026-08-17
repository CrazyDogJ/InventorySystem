// Fill out your copyright notice in the Description page of Project Settings.

#include "InventorySlotData.h"

#include "Processors/InventoryProcessor_Stackable.h"

//////////////////////////////
// UInventorySlotData_Stack //
//////////////////////////////

FInstancedStruct UInventorySlotData_Stack::GetDefaultSlotData_Implementation(
	UInventoryItemDefinition* ItemDefinition, UObject* OuterObject) const
{
	return FInstancedStruct::Make<FItemSlotData_Stack>(FItemSlotData_Stack());
}

bool UInventorySlotData_Stack::CanStack_Implementation(const FInventoryItemEntry& ItemEntry, const FInventoryItemEntry& OtherItemEntry) const
{
	// Validate
	if (ItemEntry.IsSlotEmpty() || OtherItemEntry.IsSlotEmpty())
	{
		return false;
	}
	
	if (ItemEntry.ItemDefinition != OtherItemEntry.ItemDefinition)
	{
		return false;
	}
	
	// Default
	const auto DefaultMaxStack = GetMaxStackAmount(ItemEntry);
	const auto ToMax = DefaultMaxStack - GetStackAmount(ItemEntry);
	return ToMax >= 1;
}

int UInventorySlotData_Stack::GetStackAmount_Implementation(const FInventoryItemEntry& ItemEntry) const
{
	if (const auto StackData = ItemEntry.ItemSlotData.GetPtr<FItemSlotData_Stack>())
	{
		return StackData->StackAmount;
	}

	if (ItemEntry.ItemDefinition)
	{
		return 1;
	}

	return Super::GetStackAmount_Implementation(ItemEntry);
}

int UInventorySlotData_Stack::GetMaxStackAmount_Implementation(const FInventoryItemEntry& ItemEntry) const
{
	const auto ItemDef = ItemEntry.ItemDefinition;
	if (!ItemDef)
	{
		return INDEX_NONE;
	}
	
	if (ItemEntry.ItemSlotFilter.bOnlyOne)
	{
		return 1;
	}

	// Will get if existed.
	if (const auto StackablePtr = ItemDef->ItemDefaultFragments.GetFragmentPtr<FItemFragment_Stackable>())
	{
		return StackablePtr->MaxStackAmount;
	}

	return Super::GetMaxStackAmount_Implementation(ItemEntry);
}

void UInventorySlotData_Stack::SetStackAmount_Implementation(FInventoryItemEntry& ItemEntry, int NewAmount) const
{
	if (const auto StackData = ItemEntry.ItemSlotData.GetMutablePtr<FItemSlotData_Stack>())
	{
		StackData->StackAmount = FMath::Clamp(NewAmount, 1, GetMaxStackAmount(ItemEntry));
	}
}

void UInventorySlotData_Stack::ValidateStackAmount_Implementation(FInventoryItemEntry& ItemEntry) const
{
	if (const auto StackData = ItemEntry.ItemSlotData.GetMutablePtr<FItemSlotData_Stack>())
	{
		StackData->StackAmount = FMath::Clamp(StackData->StackAmount, 1, GetMaxStackAmount(ItemEntry));
	}
}

bool UInventorySlotData_Stack::AddStackAmount_Implementation(FInventoryItemEntry& ItemEntry,
                                                             FInventoryItemEntry& InItemEntry, int SpecificAmount) const
{
	const auto StackData = ItemEntry.ItemSlotData.GetMutablePtr<FItemSlotData_Stack>();
	const auto InStackData = InItemEntry.ItemSlotData.GetMutablePtr<FItemSlotData_Stack>();
	if (StackData && InStackData)
	{
		const auto OldStackAmount = StackData->StackAmount;
		StackData->StackAmount = FMath::Clamp(OldStackAmount + (SpecificAmount > 0 ?
			FMath::Min(SpecificAmount, InStackData->StackAmount) : InStackData->StackAmount), 0, GetMaxStackAmount(ItemEntry));
		
		const auto ChangedAmount = StackData->StackAmount - OldStackAmount;
		InStackData->StackAmount -= ChangedAmount;
		if (InStackData->StackAmount <= 0)
		{
			InItemEntry.EmptySlot();
			return true;
		}
	}

	return false;
}

FInventoryItemEntry UInventorySlotData_Stack::RemoveStackAmount_Implementation(FInventoryItemEntry& ItemEntry,
	int StackAmount) const
{
	const auto StackData = ItemEntry.ItemSlotData.GetMutablePtr<FItemSlotData_Stack>();
	if (StackData && StackAmount > 0)
	{
		auto NewTempEntry = ItemEntry;
		
		const auto OldStackAmount = StackData->StackAmount;
		StackData->StackAmount = FMath::Max(0, OldStackAmount - StackAmount);
		const auto ChangedAmount = OldStackAmount - StackData->StackAmount;
		
		if (StackData->StackAmount == 0)
		{
			ItemEntry.EmptySlot();
		}
		SetStackAmount(NewTempEntry, ChangedAmount);

		return NewTempEntry;
	}

	return FInventoryItemEntry();
}

void UInventorySlotData_Stack::EmptySlotData_Implementation(FInventoryItemEntry& ItemEntry) const
{
	Super::EmptySlotData_Implementation(ItemEntry);
}

FInstancedStruct UInventorySlotData_Stack::GetSlotDataSaveGame_Implementation(
	const FInventoryItemEntry& ItemEntry) const
{
	return ItemEntry.ItemSlotData;
}

void UInventorySlotData_Stack::LoadSlotData_Implementation(FInventoryItemEntry& ItemEntry,
	const FInstancedStruct& InSlotSaveGame, UObject* OuterObject) const
{
	ItemEntry.ItemSlotData = InSlotSaveGame;
}

/////////////////////////////////////
// UInventorySlotData_ItemInstance //
/////////////////////////////////////

FInstancedStruct UInventorySlotData_ItemInstance::GetDefaultSlotData_Implementation(
	UInventoryItemDefinition* ItemDefinition, UObject* OuterObject) const
{
	const auto ItemInstance = UInventoryItemInstance::NewItemInstance(OuterObject, ItemDefinition);
	auto InstancedStruct = FItemSlotData_ItemInstance(ItemInstance);
	return FInstancedStruct::Make<FItemSlotData_ItemInstance>(InstancedStruct);
}

bool UInventorySlotData_ItemInstance::CanStack_Implementation(const FInventoryItemEntry& ItemEntry,
	const FInventoryItemEntry& OtherItemEntry) const
{
	return false;
}

int UInventorySlotData_ItemInstance::GetStackAmount_Implementation(const FInventoryItemEntry& ItemEntry) const
{
	if (const auto StackData = ItemEntry.ItemSlotData.GetPtr<FItemSlotData_Stack>())
	{
		return StackData->StackAmount;
	}
	
	return ItemEntry.ItemDefinition ? 1 : 0;
}

int UInventorySlotData_ItemInstance::GetMaxStackAmount_Implementation(const FInventoryItemEntry& ItemEntry) const
{
	return 1;
}

void UInventorySlotData_ItemInstance::SetStackAmount_Implementation(FInventoryItemEntry& ItemEntry, int NewAmount) const
{
	if (NewAmount == 0)
	{
		ItemEntry.EmptySlot();
	}
}

bool UInventorySlotData_ItemInstance::AddStackAmount_Implementation(FInventoryItemEntry& ItemEntry,
	FInventoryItemEntry& InItemEntry, int SpecificAmount) const
{
	if (const auto InstanceFragPtr = ItemEntry.ItemSlotData.GetMutablePtr<FItemSlotData_ItemInstance>())
	{
		if (!InstanceFragPtr->ItemInstance)
		{
			if (const auto InInstanceFragPtr = InItemEntry.ItemSlotData.GetMutablePtr<FItemSlotData_ItemInstance>())
			{
				InstanceFragPtr->ItemInstance = InInstanceFragPtr->ItemInstance;
				return true;
			}
		}
	}

	return false;
}

FInventoryItemEntry UInventorySlotData_ItemInstance::RemoveStackAmount_Implementation(FInventoryItemEntry& ItemEntry,
	int StackAmount) const
{
	if (StackAmount >= 1)
	{
		auto TempEntry = ItemEntry;
		ItemEntry.EmptySlot();
		return TempEntry;
	}
	
	return FInventoryItemEntry();
}

void UInventorySlotData_ItemInstance::EmptySlotData_Implementation(FInventoryItemEntry& ItemEntry) const
{
	if (const auto InstanceFragPtr = ItemEntry.ItemSlotData.GetMutablePtr<FItemSlotData_ItemInstance>())
	{
		if (InstanceFragPtr->ItemInstance)
		{
			InstanceFragPtr->ItemInstance->ConditionalBeginDestroy();
			InstanceFragPtr->ItemInstance = nullptr;
		}
	}

	Super::EmptySlotData_Implementation(ItemEntry);
}

FInstancedStruct UInventorySlotData_ItemInstance::GetSlotDataSaveGame_Implementation(
	const FInventoryItemEntry& ItemEntry) const
{
	if (const auto Ptr = ItemEntry.ItemSlotData.GetPtr<FItemSlotData_ItemInstance>())
	{
		return FInstancedStruct::Make(FItemSlotDataSaveGame_ItemInstance(*Ptr));
	}

	return FInstancedStruct();
}

void UInventorySlotData_ItemInstance::LoadSlotData_Implementation(FInventoryItemEntry& ItemEntry,
	const FInstancedStruct& InSlotSaveGame, UObject* OuterObject) const
{
	ItemEntry.ItemSlotData = GetDefaultSlotData(ItemEntry.ItemDefinition, OuterObject);
	if (const auto Ptr = ItemEntry.ItemSlotData.GetMutablePtr<FItemSlotData_ItemInstance>())
	{
		if (Ptr->ItemInstance)
		{
			if (const auto SavePtr = InSlotSaveGame.GetPtr<FItemSlotDataSaveGame_ItemInstance>())
			{
				Ptr->ItemInstance->LoadSaveData(SavePtr->ItemInstanceData);
			}
		}
	}
}

#if WITH_EDITOR
bool UInventorySlotData_ItemInstance::ShouldRefreshItemEntry_Implementation(const FInventoryItemEntry& ItemEntry) const
{
	// Not valid;
	if (ItemEntry.ItemDefinition)
	{
		if (const auto ItemFrag = ItemEntry.ItemDefinition->GetFragmentPtr<FItemFragment_ItemInstance>())
		{
			if (const auto InstanceFrag = ItemEntry.ItemSlotData.GetPtr<FItemSlotData_ItemInstance>())
			{
				const auto ItemInstanceClass = ItemFrag->ItemInstance.LoadSynchronous();
				if (ItemInstanceClass && !InstanceFrag->ItemInstance)
				{
					return true;
				}

				if (InstanceFrag->ItemInstance && InstanceFrag->ItemInstance->GetClass() != ItemInstanceClass)
				{
					return true;
				}
			}
			else
			{
				return true;
			}
		}
	}

	return false;
}
#endif

void UInventorySlotData_ItemInstance::PostBeginPlay_Implementation(FInventoryItemEntry& ItemEntry,
                                                                   UObject* OuterObject) const
{
	if (const auto Ptr = ItemEntry.ItemSlotData.GetMutablePtr<FItemSlotData_ItemInstance>())
	{
		if (Ptr->ItemInstance)
		{
			Ptr->ItemInstance->ChangeOuter(OuterObject);
		}
	}
}

void UInventorySlotData_ItemInstance::TickItemEntry_Implementation(FInventoryItemEntry& ItemEntry,
                                                                   float DeltaSeconds) const
{
	if (const auto Ptr = ItemEntry.ItemSlotData.GetMutablePtr<FItemSlotData_ItemInstance>())
	{
		if (Ptr->ItemInstance && Ptr->ItemInstance->bTickable)
		{
			Ptr->ItemInstance->Tick(DeltaSeconds);
		}
	}
}
