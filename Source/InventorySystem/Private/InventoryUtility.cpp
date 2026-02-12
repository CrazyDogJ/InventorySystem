// Copyright CrazyDog. All Rights Reserved.

#include "InventoryUtility.h"

#include "InventoryItemActor.h"

TSubclassOf<AInventoryItemActor> FItemActorDescBase::GetItemActorClass() const
{
	return AInventoryItemActor::StaticClass();
}
