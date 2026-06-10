// Copyright CrazyDog. All Rights Reserved.

#include "Grid/InventoryGridContainerComponent.h"

#include "Net/UnrealNetwork.h"

UGridInventoryContainerComponent::UGridInventoryContainerComponent()
{
	// Base ctor already sets replication + registered sub-object list.
}

void UGridInventoryContainerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, GridItemList);
}

void UGridInventoryContainerComponent::BeginPlay()
{
	GridItemList.OuterObject = this;
	GridItemList.GridWidth = FMath::Max(1, InitGridWidth);
	GridItemList.GridHeight = FMath::Max(1, InitGridHeight);
	GridItemList.RebuildOccupancy();

	Super::BeginPlay();
}

void UGridInventoryContainerComponent::BindGridAddEvent(const FOnFastArraySerializerStaticEvent& Event)
{
	GridItemList.OnItemListAdd.AddLambda([Event](const TArray<int>& Indices)
	{
		Event.ExecuteIfBound(Indices);
	});
}

void UGridInventoryContainerComponent::BindGridRemoveEvent(const FOnFastArraySerializerStaticEvent& Event)
{
	GridItemList.OnItemListRemove.AddLambda([Event](const TArray<int>& Indices)
	{
		Event.ExecuteIfBound(Indices);
	});
}

void UGridInventoryContainerComponent::BindGridChangeEvent(const FOnFastArraySerializerStaticEvent& Event)
{
	GridItemList.OnItemListChange.AddLambda([Event](const TArray<int>& Indices)
	{
		Event.ExecuteIfBound(Indices);
	});
}
