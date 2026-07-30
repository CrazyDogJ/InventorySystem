// Copyright CrazyDog. All Rights Reserved.

#include "InventoryUtility.h"

#include "InventoryItemActor.h"

TSubclassOf<AInventoryItemActor> FItemActorDescBase::GetItemActorClass() const
{
	return AInventoryItemActor::StaticClass();
}

bool FItemSlotFilter::IsItemAllowed(const UInventoryItemDefinition* Item) const
{
	UInventoryItemDefinition* ItemRaw = const_cast<UInventoryItemDefinition*>(Item);
	const TSoftObjectPtr<UInventoryItemDefinition> Soft = ItemRaw;
	return IsItemAllowed(Soft);
}

bool FItemSlotFilter::IsItemAllowed(const TSoftObjectPtr<UInventoryItemDefinition>& Item) const
{
	if (AllowedItemTags.IsEmpty())
	{
		if (AllowedItems.IsEmpty())
		{
			return true;
		}

		return AllowedItems.Contains(Item);
	}

	if (Item)
	{
		return AllowedItemTags.HasTag(Item->ItemGameplayTag);
	}

	return true;
}

FItemStackMapping::FItemStackMapping(TMap<TObjectPtr<UInventoryItemDefinition>, int> InItems)
{
	for (const auto Item : InItems)
	{
		if (Item.Key && Item.Value > 0)
		{
			const auto SoftPtr = TSoftObjectPtr<UInventoryItemDefinition>(Item.Key);
			Items.Add(SoftPtr, Item.Value);
		}
	}
}

void FItemStackMapping::ChangeItemAmount(const TSoftObjectPtr<UInventoryItemDefinition>& InItemDefinition, const int DeltaAmount)
{
	if (InItemDefinition)
	{
		if (const auto FoundPtr = Items.Find(InItemDefinition))
		{
			if (*FoundPtr + DeltaAmount > 0)
			{
				const auto Amount = *FoundPtr + DeltaAmount;
				Items.Add(InItemDefinition, Amount);
				ItemStackMappingEvent.Broadcast(InItemDefinition, Amount);
			}
			else
			{
				Items.Remove(InItemDefinition);
				ItemStackMappingEvent.Broadcast(InItemDefinition, 0);
			}
		}
		else
		{
			Items.Add(InItemDefinition, DeltaAmount);
			ItemStackMappingEvent.Broadcast(InItemDefinition, DeltaAmount);
		}
	}
}
