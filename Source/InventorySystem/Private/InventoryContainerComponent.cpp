// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryContainerComponent.h"

#include "InventoryItemInstance.h"
#include "InventorySaveGame.h"
#include "InventorySystem.h"
#include "InventorySystemLibrary.h"
#include "StreamingLevelSaveComponent.h"
#include "StreamingLevelSaveLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Processors/InventoryProcessor_Stackable.h"

UInventoryContainerComponent::UInventoryContainerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UInventoryContainerComponent::ResetInventoryList()
{
	if (bShouldInit)
	{
		for (int i = 0; i < InitItemList.ItemList.Num(); ++i)
		{
			if (ItemList.ItemList.IsValidIndex(i))
			{
				// Remove all items in item list.
				ItemList.EmptySlotByIndex({i}, false);
				ItemList.ItemList[i].CopyFrom(InitItemList.ItemList[i]);
				ItemList.MarkIndexChange(i);
			}
		}
	}
}

void UInventoryContainerComponent::OnItemPickupUpClient_Implementation(const UInventoryItemDefinition* ItemDef,
                                                                       const int Amount)
{
	OnItemPickedUpEvent.Broadcast(ItemDef, Amount);
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
	if (bShouldInit && GetOwner()->HasAuthority())
	{
		for (int i = 0; i < InitItemList.ItemList.Num(); ++i)
		{
			ItemList.ItemList.Add(InitItemList.ItemList[i]);
			// Item instance.
			if (const auto ItemInstance = ItemList.ItemList[i].ItemInstance)
			{
				ItemInstance->ChangeOuter(this);
			}
			ItemList.MarkIndexAdd(i);
		}
		
	}
	
	if (bAutoStreamingSave && UStreamingLevelSaveLibrary::IsRuntimeObject(GetOwner()) && GetOwner()->HasAuthority() && !StreamingLevelSaveComponent)
	{
		const auto NewComponent = GetOwner()->AddComponentByClass(UStreamingLevelSaveComponent::StaticClass(), false, FTransform(), false);
		StreamingLevelSaveComponent = Cast<UStreamingLevelSaveComponent>(NewComponent);
	}
	
	Super::BeginPlay();
}

void UInventoryContainerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bShouldTickItem)
	{
		for (const auto Itr : ItemList.ItemList)
		{
			if (Itr.ItemInstance && Itr.ItemInstance->bTickable)
			{
				Itr.ItemInstance->Tick(DeltaTime);
			}
		}
	}
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

#if WITH_EDITOR

void UInventoryContainerComponent::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(FInventoryItemEntry, ItemStack))
	{
		const int32 Index = PropertyChangedEvent.GetArrayIndex(TEXT("ItemList"));
		if (InitItemList.ItemList.IsValidIndex(Index))
		{
			auto& ItemEntry = InitItemList.ItemList[Index];
			if (ItemEntry.ItemDefinition)
			{
				const auto InstanceFrag = ItemEntry.ItemDefinition->GetFragmentPtr<FItemFragment_ItemInstance>();
				if (InstanceFrag && InstanceFrag->ItemInstance->Implements<UStackableItem>())
				{
					ItemEntry.ItemStack = 1;
					return;
				}

				if (const auto Stackable = ItemEntry.ItemDefinition->GetFragmentPtr<FItemFragment_Stackable>())
				{
					ItemEntry.ItemStack = FMath::Clamp(ItemEntry.ItemStack, 1, Stackable->MaxStackAmount);
					return;
				}
			}
		}
	}
	
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

#endif