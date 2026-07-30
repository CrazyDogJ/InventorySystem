// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemDefinition.h"
#include "InventoryUtility.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "InventoryListContainer.generated.h"

class UInventoryItemDefinition;

/////////////////////////////////////
/// Inventory Item List Container ///
/////////////////////////////////////

/** Item list entry. Will be common used in most situation. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FInventoryItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInventoryItemEntry() {}
	FInventoryItemEntry(UObject* OuterObject, UInventoryItemDefinition* InItemDefinition, const FItemSlotFilter& InItemSlotFilter = FItemSlotFilter());

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UInventoryItemDefinition* ItemDefinition = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FInstancedStruct ItemSlotData;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FItemSlotFilter ItemSlotFilter;
	
	void CopyFrom(const FInventoryItemEntry& Other, const bool bCopyFilter = false);
	void SetItemSlotFilter(const FItemSlotFilter& Filter);
	bool IsItemAllowed(const UInventoryItemDefinition* ItemDef) const;
	bool IsSlotEmpty() const;
	void EmptySlot(const bool& bCallEmptyLogic = true, const bool& bClearFilter = false);
	int GetStackAmount() const;
	void SetStackAmount(const int& InStackAmount);
	int GetMaxStackAmount() const;
	bool CanEntryStack(const FInventoryItemEntry& OtherItemEntry) const;
	bool StackEntry(FInventoryItemEntry& OtherEntry, int SpecificAmount = INDEX_NONE);
	void RemoveStack(int SpecificAmount = 1);

	TObjectPtr<UInventoryItemInstance> TryGetItemInstance() const;
};

/** Container for fast array inventory item entries. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FInventoryItemList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FInventoryItemEntry> ItemList;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, NotReplicated)
	UObject* OuterObject = nullptr;
	
	// Empty the slot. If you want to dirty by your self, please set bDirty to false.
	void EmptySlotByIndex(TArray<int32> Indices, bool bDirty = true);
	// Use it carefully !!!
	void Clear();
	
	void AddEmptySlots(const int32 AddAmount);
	void RemoveSlots(TArray<int32> Indices);
	void MarkIndexAdd(int32 Index);
	void MarkIndexAdd(TArray<int32> Indices);
	void MarkIndexRemove(int32 Index);
	void MarkIndexRemove(TArray<int32> Indices);
	void MarkIndexChange(int32 Index);
	void MarkIndexChange(TArray<int32> Indices);

	bool IsSlotEmpty(int Index, bool& Empty) const;
	int FindFirstEmptySlot(const UInventoryItemDefinition* ItemDefinition = nullptr) const;
	int FindFirstStackableSlot(const FInventoryItemEntry& OtherItemEntry) const;
	int GetItemTotalAmountByDefinition(const TSoftObjectPtr<UInventoryItemDefinition>& ItemDefinitionSoft) const;
	int GetItemTotalAmountByDefinition(const UInventoryItemDefinition* ItemDef) const;
	UInventoryItemDefinition* GetItemDefinition(const int& Index) const;
	UInventoryItemInstance* GetItemInstance(const int& Index) const;
	bool CanSlotSplit(const int& SlotIndex) const;
	
	FItemStackMapping GetItemMappings() const;
	bool CanRemoveItems(const FItemStackMapping& InItems) const;
	bool CanAddItems(const FItemStackMapping& InItems) const;
	bool CanExchangeItems(const FItemStackMapping& RemoveItems, const FItemStackMapping& AddItems) const;
	bool ExchangeItems(const FItemStackMapping& RemoveItems, const FItemStackMapping& AddItems);

	bool RemoveStackAtIndex(const int& Index, const int& Amount, const bool& bForceRemove = false);
	int RemoveItemByDefinition(const UInventoryItemDefinition* ItemDef, const int& Amount, const bool& bForceRemove = false);
	bool AddItemByDefinition(UInventoryItemDefinition* ItemDef, const int& Amount);
	bool AddItem(FInventoryItemEntry& ItemEntry);
	bool DragDropItem(int DragIndex, FInventoryItemList& DropItemList, int DropIndex);
	bool QuickMoveItem(int FromIndex, FInventoryItemList& ToItemList);
	bool SplitItem(int SplitIndex, int SplitAmount, FInventoryItemList& ContainerToSplit);
	
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	
	FOnFastArraySerializerEvent OnItemListRemove;
	FOnFastArraySerializerEvent OnItemListAdd;
	FOnFastArraySerializerEvent OnItemListChange;
	
	// Replication
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);
};

template<> struct TStructOpsTypeTraits<FInventoryItemList> : public TStructOpsTypeTraitsBase2<FInventoryItemList>
{
	enum { WithNetDeltaSerializer = true };
};
