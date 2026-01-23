// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventorySaveGame.generated.h"

class AInventoryItemActor;
class UInventoryItemDefinition;
struct FInventoryItemList;
struct FInventoryItemEntry;
class UInventoryItemInstance;

/** Item list entry save data. */
USTRUCT()
struct FInventoryItemEntrySaveData
{
	GENERATED_BODY()
	
	void SaveEntry(const FInventoryItemEntry& InItemEntry);
	void LoadEntry(FInventoryItemEntry& InItemEntry, UObject* OuterObject) const;

	UPROPERTY(SaveGame)
	TSoftObjectPtr<UInventoryItemDefinition> ItemDefinition;
	
	UPROPERTY(SaveGame)
	TArray<uint8> ItemInstanceData;
};

/** Item list array save data. */
USTRUCT(BlueprintType)
struct FInventoryItemListSaveData
{
	GENERATED_BODY()

	void SaveList(const FInventoryItemList& InItemList);
	void LoadList(FInventoryItemList& InItemList, UObject* OuterObject) const;
	
	UPROPERTY(SaveGame)
	TArray<FInventoryItemEntrySaveData> ItemsSaveData;
};
