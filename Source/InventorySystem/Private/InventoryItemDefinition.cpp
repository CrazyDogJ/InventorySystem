// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemDefinition.h"

FInstancedStruct UInventoryItemDefinition::FindFragmentByClass(const UScriptStruct* StructType,
                                                               bool& bValid) const
{
	FInstancedStruct EmptyStruct;
	EmptyStruct.InitializeAs(StructType);

	if (const auto Found = TypeLookup.Find(StructType))
	{
		FInstancedStruct Dummy;
		Dummy.InitializeAs(DefaultFragments[*Found].GetScriptStruct());

		if (Dummy.Identical(&EmptyStruct, 0))
		{
			bValid = true;
			return DefaultFragments[*Found];
		}
	}
	
	bValid = false;
	return FInstancedStruct();
}

#if WITH_EDITOR
void UInventoryItemDefinition::RebuildLookup(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, DefaultFragments))
	{
		// Update lookup pre save.
		TypeLookup.Empty();
	
		for (int i = 0; i < DefaultFragments.Num(); i++)
		{
			if (DefaultFragments[i].IsValid())
			{
				auto Type = DefaultFragments[i].GetScriptStruct();
				TypeLookup.Add(Type, i);
			}
		}
	}
}

void UInventoryItemDefinition::RefreshLocalizationKey(const FPropertyChangedEvent& PropertyChangedEvent)
{
	const auto PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, ItemID) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, ItemDisplayName) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, ItemDisplayDescription))
	{
		if (!ItemID.IsEmpty())
		{
			ItemDisplayName =
				FText::ChangeKey(TEXT("Inventory"), ItemID + "DisplayName", ItemDisplayName);
			ItemDisplayDescription =
				FText::ChangeKey(TEXT("Inventory"), ItemID + "DisplayDescription", ItemDisplayDescription);
		}
	}
}

void UInventoryItemDefinition::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	Modify();
	RebuildLookup(PropertyChangedEvent);
	RefreshLocalizationKey(PropertyChangedEvent);

	PropertyChangedDelegate.Broadcast(PropertyChangedEvent);
}
#endif
