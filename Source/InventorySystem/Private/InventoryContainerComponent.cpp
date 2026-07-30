// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryContainerComponent.h"

#include "AbilitySystemGlobals.h"
#include "InventorySlotData.h"
#include "Net/UnrealNetwork.h"

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

void UInventoryContainerComponent::ExecuteItemAction(const int Index, const int ActionIndex, UInventoryContainerComponent* OtherContainer)
{
	if (const auto ItemDef = ItemList.GetItemDefinition(Index))
	{
		if (const auto Actions = ItemDef->GetFragmentPtr<FItemFragment_ItemActions>())
		{
			if (Actions->ItemActions.IsValidIndex(ActionIndex))
			{
				const auto ItemAction = Actions->ItemActions[ActionIndex];
				if (const auto ActionInstance = ItemAction.ActionInstance)
				{
					auto Asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
					const auto Context = FInventoryItemActionContext(this, Index, OtherContainer, Asc);
					ActionInstance->OnActionExecute(Context);
					
					if (!ItemAction.bLocalAction && !GetOwner()->HasAuthority())
					{
						ExecuteItemAction_Server(ActionInstance, Context);
					}
				}
			}
		}
	}
}

void UInventoryContainerComponent::ExecuteItemAction_Server_Implementation(UInventoryItemAction* ItemAction,
                                                                           const FInventoryItemActionContext& Context)
{
	if (ItemAction)
	{
		ItemAction->OnActionExecute(Context);
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
			if (ItemList.ItemList[i].ItemDefinition)
			{
				if (const auto CDO = ItemList.ItemList[i].ItemDefinition->GetSlotDataDefinitionCDO())
				{
					CDO->PostBeginPlay(ItemList.ItemList[i], this);
				}
			}
			ItemList.MarkIndexAdd(i);
		}
	}
	
	Super::BeginPlay();
}

void UInventoryContainerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bShouldTickItem)
	{
		for (auto& Itr : ItemList.ItemList)
		{
			if (Itr.ItemDefinition)
			{
				if (const auto CDO = Itr.ItemDefinition->GetSlotDataDefinitionCDO())
				{
					CDO->TickItemEntry(Itr, DeltaTime);
				}
			}
		}
	}
}

#if WITH_EDITOR

void UInventoryContainerComponent::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	const int32 Index = PropertyChangedEvent.GetArrayIndex(TEXT("ItemList"));
	if (InitItemList.ItemList.IsValidIndex(Index))
	{
		auto& ItemEntry = InitItemList.ItemList[Index];
		if (ItemEntry.ItemDefinition)
		{
			if (const auto CDO = ItemEntry.ItemDefinition->GetSlotDataDefinitionCDO())
			{
				CDO->ValidateStackAmount(ItemEntry);
			}
		}
	}
	
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

#endif