// Copyright CrazyDog. All Rights Reserved.

#include "InventoryUtility.h"

#include "InventoryItemActor.h"

TSubclassOf<AInventoryItemActor> FItemActorDescBase::GetItemActorClass() const
{
	return AInventoryItemActor::StaticClass();
}

bool FItemSlotFilter::IsItemAllowed(const UInventoryItemDefinition* Item) const
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
