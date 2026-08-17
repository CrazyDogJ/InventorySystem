// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemInstance.h"
#include "InventoryListContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "InventorySlotData.generated.h"

class UInventoryItemInstance;
class UInventoryItemDefinition;

/** Common slot data for stackable items. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemSlotData_Stack
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int StackAmount = 0;
};

/** Special slot data for items that need instance object to manage. ONLY STACK ONE in a single slot! */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemSlotData_ItemInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Instanced)
	TObjectPtr<UInventoryItemInstance> ItemInstance;
};

/** Base slot data class. */
UCLASS(Blueprintable, BlueprintType)
class INVENTORYSYSTEM_API UInventorySlotDataBase : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Slot Data")
	FInstancedStruct GetDefaultSlotData(UInventoryItemDefinition* ItemDefinition, UObject* OuterObject = nullptr) const;
	virtual FInstancedStruct GetDefaultSlotData_Implementation(UInventoryItemDefinition* ItemDefinition, UObject* OuterObject = nullptr) const
	{
		return FInstancedStruct();
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Slot Data")
	bool CanStack(const FInventoryItemEntry& ItemEntry, const FInventoryItemEntry& OtherItemEntry) const;
	virtual bool CanStack_Implementation(const FInventoryItemEntry& ItemEntry, const FInventoryItemEntry& OtherItemEntry) const
	{
		return false;
	}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Slot Data")
	int GetStackAmount(const FInventoryItemEntry& ItemEntry) const;
	virtual int GetStackAmount_Implementation(const FInventoryItemEntry& ItemEntry) const
	{
		return 1;
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Slot Data")
	int GetMaxStackAmount(const FInventoryItemEntry& ItemEntry) const;
	virtual int GetMaxStackAmount_Implementation(const FInventoryItemEntry& ItemEntry) const
	{
		return 1;
	}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Slot Data")
	void SetStackAmount(UPARAM(ref) FInventoryItemEntry& ItemEntry, int NewAmount) const;
	virtual void SetStackAmount_Implementation(FInventoryItemEntry& ItemEntry, int NewAmount) const {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Slot Data")
	void ValidateStackAmount(UPARAM(ref) FInventoryItemEntry& ItemEntry) const;
	virtual void ValidateStackAmount_Implementation(FInventoryItemEntry& ItemEntry) const
	{
		
	}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Slot Data")
	bool AddStackAmount(UPARAM(ref) FInventoryItemEntry& ItemEntry, UPARAM(ref) FInventoryItemEntry& InItemEntry, int SpecificAmount = 0) const;
	virtual bool AddStackAmount_Implementation(FInventoryItemEntry& ItemEntry, FInventoryItemEntry& InItemEntry, int SpecificAmount = 0) const
	{
		return false;
	}

	/** Return removed slot data. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Slot Data")
	FInventoryItemEntry RemoveStackAmount(UPARAM(ref) FInventoryItemEntry& ItemEntry, int StackAmount) const;
	virtual FInventoryItemEntry RemoveStackAmount_Implementation(FInventoryItemEntry& ItemEntry, int StackAmount) const
	{
		return FInventoryItemEntry();
	}

	/** Clear item slot data. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Slot Data")
	void EmptySlotData(UPARAM(ref) FInventoryItemEntry& ItemEntry) const;
	virtual void EmptySlotData_Implementation(FInventoryItemEntry& ItemEntry) const
	{
		ItemEntry.ItemSlotData = FInstancedStruct();
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Slot Data")
	FInstancedStruct GetSlotDataSaveGame(const FInventoryItemEntry& ItemEntry) const;
	virtual FInstancedStruct GetSlotDataSaveGame_Implementation(const FInventoryItemEntry& ItemEntry) const
	{
		return FInstancedStruct();
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Slot Data")
	void LoadSlotData(UPARAM(ref) FInventoryItemEntry& ItemEntry, const FInstancedStruct& InSlotSaveGame, UObject* OuterObject) const;
	virtual void LoadSlotData_Implementation(FInventoryItemEntry& ItemEntry, const FInstancedStruct& InSlotSaveGame, UObject* OuterObject) const
	{
		
	}
#if WITH_EDITOR
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Slot Data")
	bool ShouldRefreshItemEntry(const FInventoryItemEntry& ItemEntry) const;
	virtual bool ShouldRefreshItemEntry_Implementation(const FInventoryItemEntry& ItemEntry) const
	{
		return false;
	}
#endif
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Slot Data")
	void PostBeginPlay(UPARAM(ref) FInventoryItemEntry& ItemEntry, UObject* OuterObject) const;
	virtual void PostBeginPlay_Implementation(FInventoryItemEntry& ItemEntry, UObject* OuterObject) const
	{
		
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Slot Data")
	void TickItemEntry(UPARAM(ref) FInventoryItemEntry& ItemEntry, float DeltaSeconds) const;
	virtual void TickItemEntry_Implementation(FInventoryItemEntry& ItemEntry, float DeltaSeconds) const
	{
		
	}
};

/** Common slot data class with stack amount integer. */
UCLASS(Blueprintable, BlueprintType)
class INVENTORYSYSTEM_API UInventorySlotData_Stack : public UInventorySlotDataBase
{
	GENERATED_BODY()

public:
	virtual FInstancedStruct GetDefaultSlotData_Implementation(UInventoryItemDefinition* ItemDefinition, UObject* OuterObject = nullptr) const override;
	virtual bool CanStack_Implementation(const FInventoryItemEntry& ItemEntry, const FInventoryItemEntry& OtherItemEntry) const override;
	virtual int GetStackAmount_Implementation(const FInventoryItemEntry& ItemEntry) const override;
	virtual int GetMaxStackAmount_Implementation(const FInventoryItemEntry& ItemEntry) const override;
	virtual void SetStackAmount_Implementation(FInventoryItemEntry& ItemEntry, int NewAmount) const override;
	virtual void ValidateStackAmount_Implementation(FInventoryItemEntry& ItemEntry) const override;
	virtual bool AddStackAmount_Implementation(FInventoryItemEntry& ItemEntry, FInventoryItemEntry& InItemEntry, int SpecificAmount = 0) const override;
	virtual FInventoryItemEntry RemoveStackAmount_Implementation(FInventoryItemEntry& ItemEntry, int StackAmount) const override;
	virtual void EmptySlotData_Implementation(FInventoryItemEntry& ItemEntry) const override;
	virtual FInstancedStruct GetSlotDataSaveGame_Implementation(const FInventoryItemEntry& ItemEntry) const override;
	virtual void LoadSlotData_Implementation(FInventoryItemEntry& ItemEntry, const FInstancedStruct& InSlotSaveGame, UObject* OuterObject) const override;
};

USTRUCT(BlueprintType)
struct FItemSlotDataSaveGame_ItemInstance
{
	GENERATED_BODY()

	FItemSlotDataSaveGame_ItemInstance() {}
	explicit FItemSlotDataSaveGame_ItemInstance(UInventoryItemInstance* ItemInstance)
	{
		if (ItemInstance)
		{
			ItemInstance->GetSaveData(ItemInstanceData);
		}
	}

	explicit FItemSlotDataSaveGame_ItemInstance(const FItemSlotData_ItemInstance& ItemInstanceStruct)
	{
		if (ItemInstanceStruct.ItemInstance)
		{
			ItemInstanceStruct.ItemInstance->GetSaveData(ItemInstanceData);
		}
	}
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<uint8> ItemInstanceData;
};

/** Special slot data class with only item instance. */
UCLASS(Blueprintable, BlueprintType)
class INVENTORYSYSTEM_API UInventorySlotData_ItemInstance : public UInventorySlotDataBase
{
	GENERATED_BODY()

public:
	virtual FInstancedStruct GetDefaultSlotData_Implementation(UInventoryItemDefinition* ItemDefinition, UObject* OuterObject = nullptr) const override;
	virtual bool CanStack_Implementation(const FInventoryItemEntry& ItemEntry, const FInventoryItemEntry& OtherItemEntry) const override;
	virtual int GetStackAmount_Implementation(const FInventoryItemEntry& ItemEntry) const override;
	virtual int GetMaxStackAmount_Implementation(const FInventoryItemEntry& ItemEntry) const override;
	virtual void SetStackAmount_Implementation(FInventoryItemEntry& ItemEntry, int NewAmount) const override;
	virtual bool AddStackAmount_Implementation(FInventoryItemEntry& ItemEntry, FInventoryItemEntry& InItemEntry, int SpecificAmount = 0) const override;
	virtual FInventoryItemEntry RemoveStackAmount_Implementation(FInventoryItemEntry& ItemEntry, int StackAmount) const override;
	virtual void EmptySlotData_Implementation(FInventoryItemEntry& ItemEntry) const override;
	virtual FInstancedStruct GetSlotDataSaveGame_Implementation(const FInventoryItemEntry& ItemEntry) const override;
	virtual void LoadSlotData_Implementation(FInventoryItemEntry& ItemEntry, const FInstancedStruct& InSlotSaveGame, UObject* OuterObject) const override;
#if WITH_EDITOR
	virtual bool ShouldRefreshItemEntry_Implementation(const FInventoryItemEntry& ItemEntry) const override;
#endif
	virtual void PostBeginPlay_Implementation(FInventoryItemEntry& ItemEntry, UObject* OuterObject) const override;
	virtual void TickItemEntry_Implementation(FInventoryItemEntry& ItemEntry, float DeltaSeconds) const override;
};