// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/UserWidgetPool.h"
#include "FastArraySerializers/InventoryListContainer.h"
#include "InventoryUserWidget.generated.h"

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
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UPROPERTY(Transient)
	FUserWidgetPool WidgetPool;
	
public:
	explicit UInventoryUserWidget(const FObjectInitializer& Initializer);
	// Pointer of the list.
	FInventoryItemList* ItemListPtr = nullptr;

	// Used for sending rpc events to server. :(
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(ExposeOnSpawn))
	UInventoryContainerComponent* InventoryContainer = nullptr;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta=(ExposeOnSpawn))
	UInventoryItemInstance* OverrideItemInstance = nullptr;
	
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
	
	// Get current slot item instance.
	UFUNCTION(BlueprintPure, Category = "Inventory System|User Widget")
	UInventoryItemInstance* GetCurrentItemInstance() const;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory System|User Widget", meta=(DeterminesOutputType="InUserWidgetClass"))
	UInventoryUserWidget* CreateChildUserWidget(TSubclassOf<UInventoryUserWidget> InUserWidgetClass, int InIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|User Widget")
	void ReleaseChildUserWidget(UInventoryUserWidget* InUserWidget);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|User Widget")
	void ListAddEvent(const TArray<int>& Indices);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|User Widget")
	void ListRemoveEvent(const TArray<int>& Indices);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|User Widget")
	void ListChangeEvent(const TArray<int>& Indices);
};
