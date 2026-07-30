// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemActor.h"

#include "InventoryItemDefinition.h"
#include "InventorySaveGame.h"
#include "InventorySlotData.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

void AInventoryItemActor::OnEntryPickedUp_Implementation(int Index)
{
	NativeOnItemActorPickedUp();
	OnItemActorPickedUp();
}

AInventoryItemActor::AInventoryItemActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void AInventoryItemActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemEntry)
}

void AInventoryItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Valid item stack.
	if (ItemEntry.ItemDefinition)
	{
		if (const auto CDO = ItemEntry.ItemDefinition->GetSlotDataDefinitionCDO())
		{
			CDO->ValidateStackAmount(ItemEntry);
		}
	}
	
#if WITH_EDITOR
	// Check item instance is updated.
	if (GetWorld()->IsEditorWorld() && ItemEntry.ItemDefinition)
	{
		CheckShouldRefresh();
		ItemEntry.ItemDefinition->PropertyChangedDelegate.AddUObject(this, &ThisClass::OnItemDefPropertyChanged);
	}
#endif
	
	OnRep_ItemEntry();
}

void AInventoryItemActor::BeginPlay()
{
	Super::BeginPlay();

	// Manually add replicated sub-object when begin play.
	if (ItemEntry.ItemDefinition)
	{
		if (const auto CDO = ItemEntry.ItemDefinition->GetSlotDataDefinitionCDO())
		{
			CDO->PostBeginPlay(ItemEntry, this);
		}
	}
}

void AInventoryItemActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ItemEntry.ItemDefinition && bShouldTickItem)
	{
		if (const auto CDO = ItemEntry.ItemDefinition->GetSlotDataDefinitionCDO())
		{
			CDO->TickItemEntry(ItemEntry, DeltaSeconds);
		}
	}
}

void AInventoryItemActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (HasAuthority())
	{
		ItemEntry.EmptySlot(true);
	}
}

void AInventoryItemActor::SetItemEntry_Implementation(FInventoryItemEntry NewEntry)
{
	ItemEntry = NewEntry;
}

void AInventoryItemActor::PostLoadItemEntry()
{
	OnRep_ItemEntry();
	if (ItemEntry.ItemDefinition)
	{
		if (const auto CDO = ItemEntry.ItemDefinition->GetSlotDataDefinitionCDO())
		{
			CDO->PostBeginPlay(ItemEntry, this);
		}
	}
}

#if WITH_EDITOR
void AInventoryItemActor::RefreshItemInstance()
{
	if (ItemEntry.ItemDefinition)
	{
		Modify();

		if (const auto CDO = ItemEntry.ItemDefinition->GetSlotDataDefinitionCDO())
		{
			ItemEntry.ItemSlotData = CDO->GetDefaultSlotData(ItemEntry.ItemDefinition, this);
			CDO->ValidateStackAmount(ItemEntry);
		}
		OnRep_ItemEntry();

		// ReSharper disable once CppExpressionWithoutSideEffects
		MarkPackageDirty();
	}
}

void AInventoryItemActor::CheckShouldRefresh()
{
	if (const auto CDO = ItemEntry.ItemDefinition->GetSlotDataDefinitionCDO())
	{
		if (CDO->ShouldRefreshItemEntry(ItemEntry))
		{
			RefreshItemInstance();
		}
	}
}

void AInventoryItemActor::OnItemDefPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UInventoryItemDefinition, ItemSlotDataDefinition))
	{
		RefreshItemInstance();
	}
	
	CheckShouldRefresh();
}

void AInventoryItemActor::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
	if (PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, ItemEntry))
	{
		if (ItemEntry.ItemDefinition)
		{
			if (const auto CDO = ItemEntry.ItemDefinition->GetSlotDataDefinitionCDO())
			{
				CDO->ValidateStackAmount(ItemEntry);
			}
		}
	}
}

#endif

void AInventoryItemActor::TransToRuntimeActor()
{
	// Only persistent item actor can trans.
	if (HasAnyFlags(RF_WasLoaded) && HasAuthority() && ItemEntry.ItemDefinition && ItemEntry.ItemDefinition->ItemActorDesc.IsValid())
	{
		const auto Desc = ItemEntry.ItemDefinition->ItemActorDesc.GetPtr<FItemActorDescBase>();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Template = this;
		const FTransform Empty = FTransform::Identity;
		GetWorld()->SpawnActor(Desc->GetItemActorClass(), &Empty, SpawnParams);
		Destroy(true);
	}
}

void AInventoryItemActor::OnRep_ItemEntry()
{
	// Client change outer here.
	if (ItemEntry.ItemDefinition)
	{
		if (const auto CDO = ItemEntry.ItemDefinition->GetSlotDataDefinitionCDO())
		{
			CDO->PostBeginPlay(ItemEntry, this);
		}
	}
}
