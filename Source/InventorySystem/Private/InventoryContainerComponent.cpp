// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryContainerComponent.h"

#include "InventoryItemInstance.h"
#include "InventorySaveGame.h"
#include "InventorySystem.h"
#include "InventorySystemLibrary.h"
#include "StreamingLevelSaveComponent.h"
#include "StreamingLevelSaveLibrary.h"
#include "Net/UnrealNetwork.h"

UInventoryContainerComponent::UInventoryContainerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UInventoryContainerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, ItemList);
}

void UInventoryContainerComponent::BeginPlay()
{
	// Init item list.
	ItemList.OuterObject = this;
	if (bShouldInit)
	{
		TArray<int32> AddIndices;
		for (int i = 0; i < InitItemList.ItemList.Num(); ++i)
		{
			ItemList.ItemList.Add(InitItemList.ItemList[i]);
			// Item instance.
			if (const auto ItemInstance = ItemList.ItemList[i].ItemInstance)
			{
				ItemInstance->ChangeOuter(this);
			}
			AddIndices.Add(i);
		}
		ItemList.MarkIndexDirty(AddIndices);
	}
	
	if (bAutoStreamingSave && UStreamingLevelSaveLibrary::IsRuntimeObject(GetOwner()) && GetOwner()->HasAuthority() && !StreamingLevelSaveComponent)
	{
		const auto NewComponent = GetOwner()->AddComponentByClass(UStreamingLevelSaveComponent::StaticClass(), false, FTransform(), false);
		StreamingLevelSaveComponent = Cast<UStreamingLevelSaveComponent>(NewComponent);
	}
	
	Super::BeginPlay();
}

FInstancedStruct UInventoryContainerComponent::GetSaveData_Implementation()
{
	FInventoryItemListSaveData SaveData;
	SaveData.SaveList(ItemList);
	return FInstancedStruct::Make(SaveData);
}

void UInventoryContainerComponent::LoadSaveData_Implementation(const FInstancedStruct& SaveData)
{
	if (const auto Ptr = SaveData.GetPtr<FInventoryItemListSaveData>())
	{
		Ptr->LoadList(ItemList, this);
		UE_LOG(LogInventorySystem, Log, TEXT("Loading save data for container : %s"), *GetName())
	}
}
