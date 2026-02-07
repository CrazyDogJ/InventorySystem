// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryUtility.h"
#include "FastArraySerializers/InventoryListContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventorySystemLibrary.generated.h"

class UInventoryUserWidget;
class UInventoryItemProcessor;
class UInventoryContainerComponent;

/**
 * Inventory system blueprint function library.
 */
UCLASS()
class INVENTORYSYSTEM_API UInventorySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Inventory System|Instanced Struct")
	static const UScriptStruct* GetInstancedStructType(const FInstancedStruct InstancedStruct);

	UFUNCTION(BlueprintCallable, Category="Inventory System|Instanced Struct")
	static int FindInstanceStructByType(TArray<FInstancedStruct> InstanceStructs, UScriptStruct* StructType);

	UFUNCTION(BlueprintCallable, Category="Inventory System|Inventory Manage", BlueprintAuthorityOnly)
	static UInventoryItemInstance* NewItemInstance(UObject* Outer, UInventoryItemDefinition* ItemDefinition);
	
	UFUNCTION(BlueprintCallable, Category="Inventory System|Inventory Manage", BlueprintAuthorityOnly)
	static FInventoryItemEntry NewItemInstanceEntry(UObject* Outer, UInventoryItemDefinition* ItemDefinition);

	// Mark item at index dirty after you change data in a slot.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manage")
	static void MarkIndexDirty(UPARAM(ref)FInventoryItemList& ItemList, TArray<int> Indices);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manage")
	static void AddEmptySlots(UPARAM(ref)FInventoryItemList& ItemList, int Amount);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manage")
	static void RemoveSlots(UPARAM(ref)FInventoryItemList& ItemList, TArray<int32> Indices);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory System|Inventory Manage")
	static void EmptySlots(UPARAM(ref)FInventoryItemList& ItemList, TArray<int32> Indices);
	
	/** Will return nullptr if slot has no item instance. */
	UFUNCTION(BlueprintCallable, Category = "Inventory System|Inventory Manage")
	static UInventoryItemInstance* GetItemInstance(UPARAM(ref)FInventoryItemList& ItemList, int Index);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|Inventory Manage")
	static bool IsSlotEmpty(UPARAM(ref)FInventoryItemList& ItemList, int Index);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|Inventory Manage")
	static int FindFirstEmptySlot(const FInventoryItemList& ItemList);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory System|Item List Events")
	static void AssignItemAddEvent(UPARAM(ref)FInventoryItemList& ItemList, FOnFastArraySerializerStaticEvent AddEvent);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|Item List Events")
	static void AssignItemRemoveEvent(UPARAM(ref)FInventoryItemList& ItemList, FOnFastArraySerializerStaticEvent AddEvent);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|Item List Events")
	static void AssignItemChangeEvent(UPARAM(ref)FInventoryItemList& ItemList, FOnFastArraySerializerStaticEvent AddEvent);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|User Widget", meta=(DeterminesOutputType="InUserWidgetClass"))
	static UInventoryUserWidget* CreateInventoryUserWidget(APlayerController* OwningController, TSubclassOf<UInventoryUserWidget> InUserWidgetClass,
		UInventoryContainerComponent* InItemListComp, int Index);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|User Widget")
	static TSubclassOf<UInventoryUserWidget> GetUserWidgetClassByInterface(TSubclassOf<UInterface> InInterface);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|User Widget")
	static TArray<TSubclassOf<UInventoryUserWidget>> GetUserWidgetsByItemInstance(UInventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintPure, Category = "Inventory System|Container Component")
	static FString GetAssociatedLevelName(const AActor* Actor);
	
	static void GetCurrentOverlappedCell(const UObject* WorldContextObject, const FVector& Location, const UWorldPartitionRuntimeCell*& CurrentOverlappedCell);
};
