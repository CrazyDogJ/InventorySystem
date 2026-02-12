// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryUserWidget.h"
#include "InventoryContainerComponent.h"
#include "InventorySubsystem.h"

void UInventoryUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ManageDelegates(true);
}

void UInventoryUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	ManageDelegates(false);
}

FInventoryItemList& UInventoryUserWidget::GetItemList() const
{
	return *ItemListPtr;
}

void UInventoryUserWidget::GetCurrentItemEntry(FInventoryItemEntry& OutItemEntry) const
{
	if (ItemListPtr)
	{
		if (ItemListPtr->ItemList.IsValidIndex(Index))
		{
			OutItemEntry = ItemListPtr->ItemList[Index];
			return;
		}
	}

	OutItemEntry = OverrideItemEntry;
}

UInventorySubsystem* UInventoryUserWidget::GetOwningInventorySubsystem() const
{
	if (GetOwningPlayer())
	{
		return GetOwningPlayer()->GetLocalPlayer()->GetSubsystem<UInventorySubsystem>();
	}
	return nullptr;
}

void UInventoryUserWidget::ManageDelegates(const bool& bManage)
{
	if (!ItemListPtr) return;
	if (bManage)
	{
		if (bAssignAddEvent)
			ItemListPtr->OnItemListAdd.AddUObject(this, &ThisClass::ListAddEvent);
		if (bAssignRemoveEvent)
			ItemListPtr->OnItemListRemove.AddUObject(this, &ThisClass::ListRemoveEvent);
		if (bAssignChangeEvent)
			ItemListPtr->OnItemListChange.AddUObject(this, &ThisClass::ListChangeEvent);
	}
	else
	{
		if (bAssignAddEvent)
			ItemListPtr->OnItemListAdd.RemoveAll(this);
		if (bAssignRemoveEvent)
			ItemListPtr->OnItemListRemove.RemoveAll(this);
		if (bAssignChangeEvent)
			ItemListPtr->OnItemListChange.RemoveAll(this);
	}
}

void UInventoryUserWidget::InitParams(UInventoryContainerComponent* InContainer, int InIndex)
{
	InventoryContainer = InContainer;
	ItemListPtr = &InContainer->ItemList;
	Index = InIndex;
}

void UInventoryUserWidget::ClearParams()
{
	ItemListPtr = nullptr;
	InventoryContainer = nullptr;
	Index = -1;
}

void UInventoryUserWidget::NativePostInventoryInitialize()
{
	ManageDelegates(true);
	PostInventoryInitialize();
}

void UInventoryUserWidget::NativePreReleaseWidgetPool()
{
	ManageDelegates(false);
	ClearParams();
	PreReleaseWidgetPool();
}
