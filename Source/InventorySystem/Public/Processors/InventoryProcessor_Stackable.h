// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemInstance.h"
#include "FastArraySerializers/InventoryListContainer.h"
#include "InventoryProcessor_Stackable.generated.h"

struct FInventoryItemList;

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
	int GetMaxStackAmount() const;
	virtual int GetMaxStackAmount_Implementation() const { return 1; }
};

UCLASS()
class UItemInstance_Stackable : public UInventoryItemInstance, public IStackableItem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Replicated, Category = "Inventory System|Item Instance")
	int StackAmount = 0;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// Stack item interface start ---
	virtual int GetStackAmount_Implementation() const override { return StackAmount; }
	virtual bool CanStack_Implementation(const UInventoryItemInstance* ItemInstance) const override;
	virtual bool StackItemInstance_Implementation(UInventoryItemInstance*& ItemInstance, int Amount = -1) override;
	virtual int RemoveStack_Implementation(int InAmount) override;
	virtual int GetMaxStackAmount_Implementation() const override;
	// Stack item interface end ---
};

/** Stackable fragment. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemFragment_Stackable
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int MaxStackAmount = 10;
};
	
/**
 * Basic slot base item operation.
 * If you are making slot base inventory, this processor must be added!
 */
UCLASS()
class INVENTORYSYSTEM_API UItemProcessor_Stackable : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:

	static bool IsStackableItem(const UInventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|Stackable")
	static int GetDefaultMaxStackAmount(UInventoryItemDefinition* ItemDef);
	
	UFUNCTION(BlueprintPure, Category = "Inventory System|Stackable")
	static int GetEntryStackAmount(const FInventoryItemEntry& ItemEntry);

	UFUNCTION(BlueprintPure, Category = "Inventory System|Stackable")
	static int GetEntryMaxStackAmount(const FInventoryItemEntry& ItemEntry);
	
	UFUNCTION(BlueprintPure, Category = "Inventory System|Stackable")
	static bool CanEntryStack(const FInventoryItemEntry& InItemEntry, const FInventoryItemEntry& OtherItemEntry);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory System|Stackable")
	static int FindFirstStackableSlot(const FInventoryItemList& ItemList, const FInventoryItemEntry& ItemEntry);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|Stackable")
	static bool StackItemEntry(UPARAM(ref)FInventoryItemEntry& ItemEntry, UPARAM(ref)FInventoryItemEntry& InItemEntry, int Amount = -1);
	
	// Remember to mark index dirty after calling it if item instance come from item list.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Stackable")
	static bool AddItem(UPARAM(ref)FInventoryItemList& ItemList, UPARAM(ref)FInventoryItemEntry& ItemEntry);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Stackable")
	static bool PickupItemActor(UPARAM(ref)FInventoryItemList& ItemList, AInventoryItemActor* ItemActor);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|Stackable")
	static int GetItemTotalAmountByDefinition(const FInventoryItemList& ItemList, UInventoryItemDefinition* ItemDef);

	// Return remain amount.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Stackable")
	static int RemoveItemByDefinition(UPARAM(ref)FInventoryItemList& ItemList, UInventoryItemDefinition* ItemDef, int Amount, bool bForceRemove = false);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Stackable")
	static bool DragDropItem(UPARAM(ref)FInventoryItemList& DragItemList, int DragIndex,
		UPARAM(ref)FInventoryItemList& DropItemList, int DropIndex);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Stackable")
	static bool QuickMoveItem(UPARAM(ref)FInventoryItemList& FromItemList, int FromIndex, UPARAM(ref)FInventoryItemList& ToItemList);

	UFUNCTION(BlueprintPure, Category = "Inventory System|Stackable")
	static bool CanSlotSplit(UPARAM(ref)FInventoryItemList& ItemList, int SlotIndex);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Stackable")
	static bool SplitItem(UPARAM(ref)FInventoryItemList& SplitItemList, int SplitIndex, int SplitAmount, UPARAM(ref)FInventoryItemList& ContainerToSplit);
	
	/**
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	static void TidyUpInventory(UPARAM(ref)FInventoryItemList& FromItemList);

	// amount -1 means drop all !
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	static AInventoryItemActor* DropItem(UPARAM(ref)FInventoryItemList& FromItemList, FTransform ItemActorTransform, int Index, int Amount = -1);
	*/
};
