// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FastArraySerializers/InventoryListContainer.h"
#include "UObject/Object.h"

class INVENTORYSYSTEMEDITOR_API FItemEntryPropertyTypeCustomization : public IPropertyTypeCustomization
{

public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	
	FReply OnPickDataAssetClicked(TSharedRef<IPropertyHandle> PropertyHandle, FInventoryItemEntry* CurrentEntry, UObject* CurrentOuter);
	
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
};
