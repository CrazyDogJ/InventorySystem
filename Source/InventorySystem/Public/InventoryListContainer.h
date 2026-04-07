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

////////////////////////////////
/// Stackable Item Interface ///
////////////////////////////////

UINTERFACE(MinimalAPI)
class UStackableItem : public UInterface
{
	GENERATED_BODY()
};

class INVENTORYSYSTEM_API IStackableItem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory System|Stackable")
	int GetStackAmount() const;
	virtual int GetStackAmount_Implementation() const { return 1; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory System|Stackable")
	bool CanStack(const UInventoryItemInstance* ItemInstance) const;
	virtual bool CanStack_Implementation(const UInventoryItemInstance* ItemInstance) const { return false; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory System|Stackable")
	bool StackItemInstance(UInventoryItemInstance*& ItemInstance, int Amount = -1);
	virtual bool StackItemInstance_Implementation(UInventoryItemInstance*& ItemInstance, int Amount = -1) { return false; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory System|Stackable")
	int RemoveStack(int InAmount);
	virtual int RemoveStack_Implementation(int InAmount) { return InAmount; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory System|Stackable")
	void SetStack(int InAmount);
	virtual void SetStack_Implementation(int InAmount) { }
};

/////////////////////////////////////
/// Inventory Item List Container ///
/////////////////////////////////////

USTRUCT(BlueprintType)
struct FItemStackMapping
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<UInventoryItemDefinition*, int> Items;
};

/** Item list entry. Will be common used in most situation. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FInventoryItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInventoryItemEntry() {}
	FInventoryItemEntry(UObject* OuterObject, UInventoryItemDefinition* InItemDefinition);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UInventoryItemDefinition* ItemDefinition = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int ItemStack = 0;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UInventoryItemInstance* ItemInstance = nullptr;

	void CopyFrom(const FInventoryItemEntry& Other)
	{
		ItemDefinition = Other.ItemDefinition;
		ItemStack = Other.ItemStack;
		ItemInstance = Other.ItemInstance;
	}
	
	bool IsSlotEmpty() const;
	void EmptySlot(const bool& bDestroyInstance = true);
	int GetStackAmount() const;
	void SetStackAmount(const int& InStackAmount);
	int GetMaxStackAmount() const;
	bool CanEntryStack(const FInventoryItemEntry& OtherItemEntry) const;
	bool StackEntry(FInventoryItemEntry& OtherEntry, int SpecificAmount = INDEX_NONE);
	void RemoveStack(int SpecificAmount = 1);
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
	int FindFirstEmptySlot() const;
	int FindFirstStackableSlot(const FInventoryItemEntry& OtherItemEntry) const;
	int GetItemTotalAmountByDefinition(const UInventoryItemDefinition* ItemDef) const;
	UInventoryItemDefinition* GetItemDefinition(const int& Index) const;
	UInventoryItemInstance* GetItemInstance(const int& Index) const;
	bool CanSlotSplit(const int& SlotIndex) const;
	bool CanRemoveItems(const FItemStackMapping& InItems) const;
	bool CanAddItems(const FItemStackMapping& InItems) const;

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
