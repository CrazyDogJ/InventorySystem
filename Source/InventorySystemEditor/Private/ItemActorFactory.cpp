// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemActorFactory.h"

#include "InventoryItemActor.h"
#include "InventoryItemDefinition.h"
#include "InventorySlotData.h"
#include "InventorySystemLibrary.h"
#include "Engine/Level.h"

UItemActorFactory::UItemActorFactory(const FObjectInitializer& ObjectInitializer)
{
	DisplayName = NSLOCTEXT("ActorFactory", "ItemActorFactory", "Used to spawn item actor. ");
}

FItemActorDescBase* UItemActorFactory::GetItemActorDesc(const FAssetData& AssetData)
{
	if (!AssetData.IsValid()) return nullptr;
	if (!AssetData.GetClass()) return nullptr;
	
	if (AssetData.GetClass()->IsChildOf(UInventoryItemDefinition::StaticClass()))
	{
		if (UInventoryItemDefinition* Asset = Cast<UInventoryItemDefinition>(AssetData.GetAsset()))
		{
			const auto ItemActorDesc = Asset->ItemActorDesc.GetMutablePtr<FItemActorDescBase>();
			return ItemActorDesc;
		}
	}
	
	return nullptr;
}

bool UItemActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	const auto Ptr = GetItemActorDesc(AssetData);
	if (Ptr && Ptr->GetItemActorClass() && Ptr->IsDataValid())
	{
		return true;
	}
	
	return false;
}

AActor* UItemActorFactory::SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform,
	const FActorSpawnParameters& InSpawnParams)
{
	const auto ItemDefAsset = Cast<UInventoryItemDefinition>(InAsset);
	if (!ItemDefAsset) return nullptr;
	
	const auto ItemActorDesc = ItemDefAsset->ItemActorDesc.GetMutablePtr<FItemActorDescBase>();
	if (!ItemActorDesc) return nullptr;

	// Item Actor Spawn
	const auto World = InLevel->GetWorld();
	auto Transform = InTransform;
	Transform.SetScale3D(ItemActorDesc->Scale);
	const auto NewActor = World->SpawnActor(ItemActorDesc->GetItemActorClass(), &Transform);
	
	// Item Actor Setup
	auto NewItemActor = Cast<AInventoryItemActor>(NewActor);
	UInventorySystemLibrary::SetupItemActor(NewItemActor, ItemDefAsset, false);
	
	return NewActor;
}

FString UItemActorFactory::GetDefaultActorLabel(UObject* Asset) const
{
	const auto ItemDefAsset = Cast<UInventoryItemDefinition>(Asset);
	if (!ItemDefAsset) return Super::GetDefaultActorLabel(Asset);
	
	return "Item_" + ItemDefAsset->ItemBaseInfo.Id;
}
