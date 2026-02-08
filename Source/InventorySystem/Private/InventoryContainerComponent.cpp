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
	ItemList.OuterObject = this;

	if (UStreamingLevelSaveLibrary::IsRuntimeObject(GetOwner()))
	{
		GetOwner()->AddComponentByClass(UStreamingLevelSaveComponent::StaticClass(), false, FTransform(), false);
	}
	
	Super::BeginPlay();
}

FInstancedStruct UInventoryContainerComponent::GetSaveData_Implementation()
{
	const FInventoryItemListSaveData SaveData = GetContainerSaveData();
	return FInstancedStruct::Make(SaveData);
}

void UInventoryContainerComponent::LoadSaveData_Implementation(const FInstancedStruct& SaveData)
{
	if (const auto Ptr = SaveData.GetPtr<FInventoryItemListSaveData>())
	{
		LoadSaveData(*Ptr);
	}
}

UInventoryItemInstance* UInventoryContainerComponent::GetItemInstance(int Index) const
{
	if (ItemList.ItemList.IsValidIndex(Index))
	{
		return ItemList.ItemList[Index].ItemInstance;
	}

	return nullptr;
}

void UInventoryContainerComponent::BeginModify()
{
#if WITH_EDITOR
	if (GetOwner())
	{
		GetOwner()->Modify();
	}
	Modify();
#endif
}

void UInventoryContainerComponent::DirtyPackage()
{
#if WITH_EDITOR
	if (GetOwner())
	{
		// ReSharper disable once CppExpressionWithoutSideEffects
		GetOwner()->MarkPackageDirty();
	}
	// ReSharper disable once CppExpressionWithoutSideEffects
	MarkPackageDirty();
#endif
}

FInventoryItemListSaveData UInventoryContainerComponent::GetContainerSaveData() const
{
	FInventoryItemListSaveData SaveData;
	SaveData.SaveList(ItemList);
	return SaveData;
}

void UInventoryContainerComponent::LoadSaveData(const FInventoryItemListSaveData& InSaveData)
{
	InSaveData.LoadList(ItemList, this);
	UE_LOG(LogInventorySystem, Log, TEXT("Loading save data for container : %s"), *GetName())
}
