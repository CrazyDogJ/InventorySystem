// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemDefinition.h"

#include "UObject/ObjectSaveContext.h"

FInstancedStruct UInventoryItemDefinition::FindFragmentByClass(const UScriptStruct* StructType,
                                                               bool& bValid) const
{
	FInstancedStruct emptystruct;
	FInstancedStruct dummy;
	emptystruct.InitializeAs(StructType);

	if (const auto Found = TypeLookup.Find(StructType))
	{
		dummy.InitializeAs(DefaultFragments[*Found].GetScriptStruct());

		if (dummy.Identical(&emptystruct, 0))
		{
			bValid = true;
			return DefaultFragments[*Found];
		}
	}
	
	bValid = false;
	return FInstancedStruct();
}

void UInventoryItemDefinition::RebuildLookup()
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
#if WITH_EDITOR
void UInventoryItemDefinition::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	Modify();
	RebuildLookup();
}
#endif
void UInventoryItemDefinition::PreSave(FObjectPreSaveContext SaveContext)
{
	Modify();
	RebuildLookup();
	
	Super::PreSave(SaveContext);
}
