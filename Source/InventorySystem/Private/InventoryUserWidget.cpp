// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryUserWidget.h"

#include "InventorySystem.h"

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

void UInventoryUserWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	WidgetPool.ReleaseAllSlateResources();
	
	Super::ReleaseSlateResources(bReleaseChildren);
}

UInventoryUserWidget::UInventoryUserWidget(const FObjectInitializer& Initializer)
	: Super(Initializer), WidgetPool(*this)
{
}

FInventoryItemList& UInventoryUserWidget::GetItemList() const
{
	return *ItemListPtr;
}

UInventoryItemInstance* UInventoryUserWidget::GetCurrentItemInstance() const
{
	// Use override first.
	if (OverrideItemInstance)
	{
		return OverrideItemInstance;
	}
	
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
	const auto NewPoolWidget = WidgetPool.GetOrCreateInstance<UInventoryUserWidget>(InUserWidgetClass,
		[this, InIndex] (UUserWidget* WidgetObject, const TSharedRef<SWidget>& Content)
		{
			if (const auto NewWidget = Cast<UInventoryUserWidget>(WidgetObject))
			{
				NewWidget->InventoryContainer = InventoryContainer;
				NewWidget->OverrideItemInstance = OverrideItemInstance;
				NewWidget->ItemListPtr = ItemListPtr;
				if (InIndex < 0)
				{
					NewWidget->Index = Index;
				}
				else
				{
					NewWidget->Index = InIndex;
				}
			}
			
			return SNew(SObjectWidget, WidgetObject)[Content];
		});

	return NewPoolWidget;
}

void UInventoryUserWidget::ReleaseChildUserWidget(UInventoryUserWidget* InUserWidget)
{
	InUserWidget->RemoveFromParent();
	WidgetPool.Release(InUserWidget);
}
