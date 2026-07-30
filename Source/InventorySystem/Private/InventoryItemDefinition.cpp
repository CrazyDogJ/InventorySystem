// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemDefinition.h"

#include "InventorySlotData.h"

UInventoryItemDefinition::UInventoryItemDefinition()
{
	ItemSlotDataDefinition = UInventorySlotDataBase::StaticClass();
}

UInventorySlotDataBase* UInventoryItemDefinition::GetSlotDataDefinitionCDO() const
{
	if (ItemSlotDataDefinition)
	{
		return ItemSlotDataDefinition.GetDefaultObject();
	}

	return nullptr;
}

#if WITH_EDITOR
void UInventoryItemDefinition::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	Modify();

	// Post edit change event for rebuild or refresh.
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, ItemDefaultFragments))
	{
		ItemDefaultFragments.RebuildTypeMapping();
	}
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, ItemBaseInfo))
	{
		ItemBaseInfo.RefreshLocalizationKey();
	}
	
	// ReSharper disable once CppExpressionWithoutSideEffects
	MarkPackageDirty();

	PropertyChangedDelegate.Broadcast(PropertyChangedEvent);
}
#endif
