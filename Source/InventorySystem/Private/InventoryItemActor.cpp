// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemActor.h"

#include "InventoryItemDefinition.h"
#include "InventorySaveGame.h"
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

	DOREPLIFETIME(ThisClass, ItemEntry)
}

void AInventoryItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Valid item stack.
	const auto MaxStack = UItemProcessor_Stackable::GetEntryMaxStackAmount(ItemEntry);
	ItemEntry.ItemStack = FMath::Clamp(ItemEntry.ItemStack, 1, MaxStack);
	
#if WITH_EDITOR
	// Check item instance is updated.
	if (GetWorld()->IsEditorWorld() && ItemEntry.ItemDefinition)
	{
		CheckShouldRefresh();
		ItemEntry.ItemDefinition->PropertyChangedDelegate.AddUObject(this, &ThisClass::OnItemDefPropertyChanged);
	}
#endif
	
	OnRep_ItemEntry();

	if (UStreamingLevelSaveLibrary::IsRuntimeObject(this) && HasAuthority() && !LevelSaveComponent)
	{
		const auto NewComp = AddComponentByClass(UStreamingLevelSaveComponent::StaticClass(), false, FTransform(), false);
		LevelSaveComponent = Cast<UStreamingLevelSaveComponent>(NewComp);
		LevelSaveComponent->bTickCheckCell = true;
	}
}

void AInventoryItemActor::BeginPlay()
{
	Super::BeginPlay();

	// Manually add replicated sub-object when begin play.
	if (ItemEntry.ItemInstance)
	{
		AddReplicatedSubObject(ItemEntry.ItemInstance);
	}
}

void AInventoryItemActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Remove item instance for streaming level. Otherwise, will cause crash when back to world.
	if (ItemEntry.ItemInstance)
	{
		RemoveReplicatedSubObject(ItemEntry.ItemInstance);
		ItemEntry.ItemInstance = nullptr;
	}
}

FInstancedStruct AInventoryItemActor::GetSaveData_Implementation()
{
	FInventoryItemEntrySaveData SaveData;
	SaveData.SaveEntry(ItemEntry);
	return FInstancedStruct::Make(SaveData);
}

void AInventoryItemActor::LoadSaveData_Implementation(const FInstancedStruct& SaveData)
{
	if (const auto Ptr = SaveData.GetPtr<FInventoryItemEntrySaveData>())
	{
		Ptr->LoadEntry(ItemEntry, this);
		OnRep_ItemEntry();
		if (ItemEntry.ItemInstance)
		{
			AddReplicatedSubObject(ItemEntry.ItemInstance);
		}
	}
}

#if WITH_EDITOR
void AInventoryItemActor::RefreshItemInstance()
{
	if (ItemEntry.ItemDefinition)
	{
		Modify();
		
		ItemEntry.ItemInstance = UInventoryItemInstance::NewItemInstance(this, ItemEntry.ItemDefinition);
		OnRep_ItemEntry();

		// ReSharper disable once CppExpressionWithoutSideEffects
		MarkPackageDirty();
	}
}

void AInventoryItemActor::CheckShouldRefresh()
{
	// Not valid;
	if (ItemEntry.ItemDefinition)
	{
		if (const auto ItemFrag = ItemEntry.ItemDefinition->GetFragmentPtr<FItemFragment_ItemInstance>())
		{
			if (ItemFrag->ItemInstance && !ItemEntry.ItemInstance)
			{
				RefreshItemInstance();
				return;
			}
		}
	}
	
	// Not the same class.
	if (const auto Def = ItemEntry.ItemDefinition)
	{
		if (const auto Ptr = Def->GetFragmentPtr<FItemFragment_ItemInstance>())
		{
			if (Ptr->ItemInstance->GetClass() != ItemEntry.ItemInstance->GetClass())
			{
				RefreshItemInstance();
				return;
			}
		}
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
	if (!UStreamingLevelSaveLibrary::IsRuntimeObject(this) && HasAuthority() && ItemEntry.ItemDefinition && ItemEntry.ItemDefinition->ItemActorDesc.IsValid())
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
{}
