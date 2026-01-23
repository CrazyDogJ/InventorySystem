// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryUtility.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "InventoryListContainer.generated.h"

struct FInventoryItemList;
class UInventoryItemDefinition;
class UInventoryItemInstance;

/** Item list entry. Will be common used in most situation. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FInventoryItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInventoryItemEntry() {}
	explicit FInventoryItemEntry(UObject* Outer, UInventoryItemDefinition* ItemDefinition);
	
	// Item instance of this slot.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Instanced)
	UInventoryItemInstance* ItemInstance = nullptr;
	
	bool IsSlotEmpty() const;
	void EmptySlot();
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

	void Clear();
	
	void AddEmptySlots(const int32 AddAmount);
	void RemoveSlots(TArray<int32> Indices);
	void MarkIndexDirty(TArray<int32> Indices);
	
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	
	FOnFastArraySerializerEvent OnItemListRemove;
	FOnFastArraySerializerEvent OnItemListAdd;
	FOnFastArraySerializerEvent OnItemListChange;
	
	// Replication
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);
};

template<> struct TStructOpsTypeTraits<FInventoryItemList> : public TStructOpsTypeTraitsBase2<FInventoryItemList>
{
	enum { WithNetDeltaSerializer = true };
};
