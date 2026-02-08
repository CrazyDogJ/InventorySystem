// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemActor.h"

#include "InventoryItemDefinition.h"
#include "InventorySaveGame.h"
#include "InventorySystemLibrary.h"
#include "StreamingLevelSaveComponent.h"
#include "StreamingLevelSaveLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Processors/InventoryProcessor_Stackable.h"

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

	DOREPLIFETIME(ThisClass, ItemInstance)
}

void AInventoryItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	// Check item instance is updated.
	if (ItemDefinition)
	{
		CheckShouldRefresh();
		ItemDefinition->PropertyChangedDelegate.AddUObject(this, &ThisClass::OnItemDefPropertyChanged);
	}
#endif
	
	OnRep_ItemInstance();
}

void AInventoryItemActor::BeginPlay()
{
	Super::BeginPlay();

	// Manually add replicated sub-object when begin play.
	if (ItemInstance)
	{
		AddReplicatedSubObject(ItemInstance);
	}
	if (UStreamingLevelSaveLibrary::IsRuntimeObject(this))
	{
		AddComponentByClass(UStreamingLevelSaveComponent::StaticClass(), false, FTransform(), false);
	}
}

FInstancedStruct AInventoryItemActor::GetSaveData_Implementation()
{
	FInventoryItemEntrySaveData SaveData;
	GetSaveData(SaveData);
	return FInstancedStruct::Make(SaveData);
}

void AInventoryItemActor::LoadSaveData_Implementation(const FInstancedStruct& SaveData)
{
	if (const auto Ptr = SaveData.GetPtr<FInventoryItemEntrySaveData>())
	{
		LoadSaveData(*Ptr);
	}
}

#if WITH_EDITOR
void AInventoryItemActor::RefreshItemInstance()
{
	if (ItemDefinition)
	{
		Modify();
		
		ItemInstance = UInventoryItemInstance::NewItemInstance(this, ItemDefinition);
		// Auto set num is 1.
		if (const auto Stackable = Cast<UItemInstance_Stackable>(ItemInstance))
		{
			Stackable->StackAmount = 1;
		}
		OnRep_ItemInstance();

		// ReSharper disable once CppExpressionWithoutSideEffects
		MarkPackageDirty();
	}
}

void AInventoryItemActor::CheckShouldRefresh()
{
	if (ItemInstance)
	{
		if (ItemDefinition->ItemInstance->GetClass() != ItemInstance->GetClass())
		{
			RefreshItemInstance();
		}
	}
	else
	{
		RefreshItemInstance();
	}
}

void AInventoryItemActor::OnItemDefPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
	CheckShouldRefresh();
}

#endif

void AInventoryItemActor::NotifyItemActorPickedUp()
{
	NativeOnItemActorPickedUp();
	OnItemActorPickedUp();
}

void AInventoryItemActor::TransToRuntimeActor()
{
	// Only persistent item actor can trans.
	if (!UStreamingLevelSaveLibrary::IsRuntimeObject(this) && HasAuthority())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Template = this;
		GetWorld()->SpawnActor(StaticClass(), &GetActorTransform(), SpawnParams);
		Destroy(true);
	}
}

void AInventoryItemActor::GetSaveData(FInventoryItemEntrySaveData& OutSaveData) const
{
	// Get entry data.
	FInventoryItemEntry NewEntry;
	NewEntry.ItemInstance = ItemInstance;
	OutSaveData.SaveEntry(NewEntry);
}

void AInventoryItemActor::LoadSaveData(const FInventoryItemEntrySaveData& InSaveData)
{
	FInventoryItemEntry NewEntry;
	InSaveData.LoadEntry(NewEntry, this);
	ItemInstance = NewEntry.ItemInstance;
	OnRep_ItemInstance();
}

void AInventoryItemActor::OnRep_ItemInstance()
{
	if (ItemInstance && ItemInstance->ItemDefinition)
	{
		if (const auto DescPtr = ItemInstance->ItemDefinition->ItemActorDesc.GetMutablePtr<>())
		{
			DescPtr->SetupActor(this);
		}
	}
}
