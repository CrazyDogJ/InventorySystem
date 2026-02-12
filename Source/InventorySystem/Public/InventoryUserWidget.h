// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FastArraySerializers/InventoryListContainer.h"
#include "InventoryUserWidget.generated.h"

class UInventorySubsystem;
class UInventoryContainerComponent;
/**
 * Used to display something about the inventory.
 */
UCLASS(Blueprintable, BlueprintType)
class INVENTORYSYSTEM_API UInventoryUserWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
public:
	// Pointer of the list.
	FInventoryItemList* ItemListPtr = nullptr;
	
	// Used for sending rpc events to server. :(
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(ExposeOnSpawn))
	UInventoryContainerComponent* InventoryContainer = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FInventoryItemEntry OverrideItemEntry = FInventoryItemEntry();
	
	// Slot index of this widget.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(ExposeOnSpawn))
	int Index = -1;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool bAssignAddEvent = false;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool bAssignRemoveEvent = false;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool bAssignChangeEvent = false;
	
public:
	UFUNCTION(BlueprintPure, Category = "Inventory System|User Widget")
	UPARAM(ref)FInventoryItemList& GetItemList() const;

	UFUNCTION(BlueprintPure, Category = "Inventory System|User Widget")
	void GetCurrentItemEntry(FInventoryItemEntry& OutItemEntry) const;

	// Get current widget's inventory subsystem.
	UFUNCTION(BlueprintPure, Category = "Inventory System|User Widget")
	UInventorySubsystem* GetOwningInventorySubsystem() const;

	void ManageDelegates(const bool& bManage);
	void InitParams(UInventoryContainerComponent* InContainer, int InIndex = -1);
	void ClearParams();

	virtual void NativePostInventoryInitialize();
	virtual void NativePreReleaseWidgetPool();
	
	/**
	 * This event will be called when widget is out of widget pool.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|User Widget")
	void PostInventoryInitialize();

	/**
	 * This event will be called before widget release back to widget pool.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|User Widget")
	void PreReleaseWidgetPool();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|User Widget")
	void ListAddEvent(const TArray<int>& Indices);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|User Widget")
	void ListRemoveEvent(const TArray<int>& Indices);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|User Widget")
	void ListChangeEvent(const TArray<int>& Indices);
};
