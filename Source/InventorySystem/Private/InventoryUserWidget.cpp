// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryUserWidget.h"
#include "InventoryContainerComponent.h"

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
	return IsValid(InventoryContainer) ? InventoryContainer->ItemList : const_cast<FInventoryItemList&>(UnboundItemList);
}

bool UInventoryUserWidget::GetCurrentItemEntry(FInventoryItemEntry& OutItemEntry) const
{
	if (IsValid(InventoryContainer))
	{
		if (InventoryContainer->ItemList.ItemList.IsValidIndex(Index))
		{
			OutItemEntry = InventoryContainer->ItemList.ItemList[Index];
			return true;
		}
	}

	OutItemEntry = OverrideItemEntry;
	return false;
}

void UInventoryUserWidget::ManageDelegates(const bool& bManage)
{
	// 修复：容器销毁或控件复用时，不继续访问旧结构体地址。
	if (!IsValid(InventoryContainer)) return;
	ItemListPtr = &InventoryContainer->ItemList;
	ItemListPtr->OnItemListAdd.RemoveAll(this);
	ItemListPtr->OnItemListRemove.RemoveAll(this);
	ItemListPtr->OnItemListChange.RemoveAll(this);
	if (bManage)
	{
		if (bAssignAddEvent)
			ItemListPtr->OnItemListAdd.AddUObject(this, &ThisClass::ListAddEvent);
		if (bAssignRemoveEvent)
			ItemListPtr->OnItemListRemove.AddUObject(this, &ThisClass::ListRemoveEvent);
		if (bAssignChangeEvent)
			ItemListPtr->OnItemListChange.AddUObject(this, &ThisClass::ListChangeEvent);
	}
}

void UInventoryUserWidget::InitParams(UInventoryContainerComponent* InContainer, int InIndex)
{
	ManageDelegates(false);
	InventoryContainer = InContainer;
	ItemListPtr = IsValid(InContainer) ? &InContainer->ItemList : nullptr;
	Index = InIndex;
}

void UInventoryUserWidget::InitParamsPoolWidget(UInventoryContainerComponent* InContainer, int InIndex)
{
	InitParams(InContainer, InIndex);
	NativePostInventoryInitialize();
}

void UInventoryUserWidget::ClearParams()
{
	ManageDelegates(false);
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
	ClearParams();
	PreReleaseWidgetPool();
}
