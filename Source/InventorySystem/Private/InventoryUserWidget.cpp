// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryUserWidget.h"

void UInventoryUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemListPtr)
	{
		if (bAssignAddEvent)
		ItemListPtr->OnItemListAdd.AddUObject(this, &ThisClass::ListAddEvent);
		if (bAssignRemoveEvent)
		ItemListPtr->OnItemListRemove.AddUObject(this, &ThisClass::ListRemoveEvent);
		if (bAssignChangeEvent)
		ItemListPtr->OnItemListChange.AddUObject(this, &ThisClass::ListChangeEvent);
	}
}

void UInventoryUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (ItemListPtr)
	{
		if (bAssignAddEvent)
		ItemListPtr->OnItemListAdd.RemoveAll(this);
		if (bAssignRemoveEvent)
		ItemListPtr->OnItemListRemove.RemoveAll(this);
		if (bAssignChangeEvent)
		ItemListPtr->OnItemListChange.RemoveAll(this);
	}

	ItemListPtr = nullptr;
}

FInventoryItemList& UInventoryUserWidget::GetItemList() const
{
	return *ItemListPtr;
}

UInventoryItemInstance* UInventoryUserWidget::GetCurrentItemInstance() const
{
	if (ItemListPtr)
	{
		if (ItemListPtr->ItemList.IsValidIndex(Index))
		{
			return ItemListPtr->ItemList[Index].ItemInstance;
		}
	}

	return nullptr;
}

UInventoryUserWidget* UInventoryUserWidget::CreateChildUserWidget(TSubclassOf<UInventoryUserWidget> InUserWidgetClass, int InIndex)
{
	if (const auto NewWidget = CreateWidget<UInventoryUserWidget>(GetOwningPlayer(), InUserWidgetClass))
	{
		NewWidget->InventoryContainer = InventoryContainer;
		NewWidget->ItemListPtr = ItemListPtr;
		if (InIndex < 0)
		{
			NewWidget->Index = Index;
		}
		else
		{
			NewWidget->Index = InIndex;
		}
	
		return NewWidget;
	}

	return nullptr;
}
