// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemRecipe.h"

#if WITH_EDITOR
void UInventoryItemRecipe::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, BaseInfo))
	{
		BaseInfo.RefreshLocalizationKey();
	}
}
#endif