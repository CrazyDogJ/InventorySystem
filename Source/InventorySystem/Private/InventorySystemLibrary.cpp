// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystemLibrary.h"

#include "InventoryContainerComponent.h"
#include "InventoryItemDefinition.h"
#include "InventoryItemInstance.h"
#include "InventorySystemSettings.h"
#include "InventoryUserWidget.h"
#include "Blueprint/UserWidget.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionRuntimeCell.h"
#include "WorldPartition/WorldPartitionRuntimeHash.h"

const UScriptStruct* UInventorySystemLibrary::GetInstancedStructType(const FInstancedStruct InstancedStruct)
{
	return InstancedStruct.GetScriptStruct();
}

UInventoryItemInstance* UInventorySystemLibrary::NewItemInstance(UObject* Outer,
	UInventoryItemDefinition* ItemDefinition)
{
	return UInventoryItemInstance::NewItemInstance(Outer, ItemDefinition);
}

FInventoryItemEntry UInventorySystemLibrary::NewItemInstanceEntry(UObject* Outer, UInventoryItemDefinition* ItemDefinition)
{
	return FInventoryItemEntry(Outer, ItemDefinition);
}

int UInventorySystemLibrary::FindInstanceStructByType(TArray<FInstancedStruct> InstanceStructs,
                                                      UScriptStruct* StructType)
{
	return InstanceStructs.IndexOfByPredicate([StructType](const FInstancedStruct& Item)
	{
		return Item.GetScriptStruct() == StructType;
	});
}

void UInventorySystemLibrary::MarkIndexDirty(FInventoryItemList& ItemList, TArray<int> Indices)
{
	ItemList.MarkIndexDirty(Indices);
}

void UInventorySystemLibrary::AddEmptySlots(FInventoryItemList& ItemList, int Amount)
{
	ItemList.AddEmptySlots(Amount);
}

void UInventorySystemLibrary::RemoveSlots(FInventoryItemList& ItemList, TArray<int32> Indices)
{
	ItemList.RemoveSlots(Indices);
}

void UInventorySystemLibrary::EmptySlots(FInventoryItemList& ItemList, TArray<int32> Indices)
{
	ItemList.EmptySlotByIndex(Indices, true);
}

UInventoryItemInstance* UInventorySystemLibrary::GetItemInstance(FInventoryItemList& ItemList, int Index)
{
	if (ItemList.ItemList.IsValidIndex(Index))
	{
		return ItemList.ItemList[Index].ItemInstance;
	}

	return nullptr;
}

bool UInventorySystemLibrary::IsSlotEmpty(FInventoryItemList& ItemList, int Index)
{
	if (ItemList.ItemList.IsValidIndex(Index))
	{
		return ItemList.ItemList[Index].IsSlotEmpty();
	}
	
	return false;
}

int UInventorySystemLibrary::FindFirstEmptySlot(const FInventoryItemList& ItemList)
{
	for (int i = 0; i < ItemList.ItemList.Num(); ++i)
	{
		if (ItemList.ItemList[i].IsSlotEmpty())
		{
			return i;
		}
	}
	
	return INDEX_NONE;
}

void UInventorySystemLibrary::AssignItemAddEvent(FInventoryItemList& ItemList,
                                                 FOnFastArraySerializerStaticEvent AddEvent)
{
	ItemList.OnItemListAdd.AddUFunction(AddEvent.GetUObject(), AddEvent.GetFunctionName());
}

void UInventorySystemLibrary::AssignItemRemoveEvent(FInventoryItemList& ItemList,
	FOnFastArraySerializerStaticEvent AddEvent)
{
	ItemList.OnItemListRemove.AddUFunction(AddEvent.GetUObject(), AddEvent.GetFunctionName());
}

void UInventorySystemLibrary::AssignItemChangeEvent(FInventoryItemList& ItemList,
	FOnFastArraySerializerStaticEvent AddEvent)
{
	ItemList.OnItemListChange.AddUFunction(AddEvent.GetUObject(), AddEvent.GetFunctionName());
}

UInventoryUserWidget* UInventorySystemLibrary::CreateInventoryUserWidget(
	APlayerController* OwningController, TSubclassOf<UInventoryUserWidget> InUserWidgetClass,
	UInventoryContainerComponent* InItemListComp, const int Index)
{
	if (const auto NewWidget = CreateWidget<UInventoryUserWidget>(OwningController, InUserWidgetClass))
	{
		NewWidget->InventoryContainer = InItemListComp;
		NewWidget->ItemListPtr = &InItemListComp->ItemList;
		NewWidget->Index = Index;
		return NewWidget;
	}
	
	return nullptr;
}

TSubclassOf<UInventoryUserWidget> UInventorySystemLibrary::GetUserWidgetClassByInterface(
	TSubclassOf<UInterface> InInterface)
{
	if (const auto Settings = GetMutableDefault<UInventorySystemSettings>())
	{
		return Settings->GetUserWidgetClass(InInterface);
	}

	return nullptr;
}

TArray<TSubclassOf<UInventoryUserWidget>> UInventorySystemLibrary::GetUserWidgetsByItemInstance(
	UInventoryItemInstance* InItemInstance)
{
	TArray<TSubclassOf<UInventoryUserWidget>> Out;
	if (!InItemInstance) return Out;
	if (const auto Settings = GetMutableDefault<UInventorySystemSettings>())
	{
		for (auto Itr : Settings->GetAllUserWidgets())
		{
			if (InItemInstance->GetClass() &&
				InItemInstance->GetClass()->ImplementsInterface(Itr.Key.LoadSynchronous()))
			{
				Out.Add(Itr.Value.LoadSynchronous());
			}
		}
	}

	return Out;
}

FString UInventorySystemLibrary::GetAssociatedLevelName(const AActor* Actor)
{
	if (!Actor) return FString();
	
	if (const ULevel* Level = Actor->GetLevel())
	{
		if (Level->GetWorldPartitionRuntimeCell())
		{
			return Level->GetWorldPartitionRuntimeCell()->GetDebugName();
		}
	}

	const UWorldPartitionRuntimeCell* Cell;
	GetCurrentOverlappedCell(Actor, Actor->GetActorLocation(), Cell);
	if (Cell != nullptr)
	{
		return Cell->GetDebugName();
	}

	return FString();
}

void UInventorySystemLibrary::GetCurrentOverlappedCell(const UObject* WorldContextObject, const FVector& Location,
                                                       const UWorldPartitionRuntimeCell*& CurrentOverlappedCell)
{
	CurrentOverlappedCell = nullptr;
	
	if (!WorldContextObject)
	{
		return;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World || !World->GetWorldPartition())
	{
		return;
	}

	const auto ForEachCellFunction = [&CurrentOverlappedCell, Location](const UWorldPartitionRuntimeCell* Cell) -> bool
	{
		if (CurrentOverlappedCell)
		{
			return true;
		}
		
		if (const auto CellBounds = Cell->GetCellBounds(); CellBounds.IsInsideXY(Location))
		{
			CurrentOverlappedCell = Cell;
			return true;
		}

		return false;
	};

	World->GetWorldPartition()->RuntimeHash->ForEachStreamingCells(ForEachCellFunction);
}
