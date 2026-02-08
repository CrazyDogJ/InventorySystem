// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StreamingLevelSaveInterface.h"
#include "Components/ActorComponent.h"
#include "FastArraySerializers/InventoryListContainer.h"
#include "InventoryContainerComponent.generated.h"

class AInventoryInfoActor;
struct FInventoryItemListSaveData;

/**
 * An inventory component that attach to visible actors.
 * There will be an inventory info actor for storing the inventory list.
 * You can try to access the info actor by this component.
 * But it will be nullptr if server don't want any specific connection to access it.
 */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent), Blueprintable)
class INVENTORYSYSTEM_API UInventoryContainerComponent : public UActorComponent, public IStreamingLevelSaveInterface
{
	GENERATED_BODY()

protected:
	// Virtual function override
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void BeginPlay() override;

	// Streaming Level Save Interface Start --------------------
	virtual FInstancedStruct GetSaveData_Implementation() override;
	virtual void LoadSaveData_Implementation(const FInstancedStruct& SaveData) override;
	// Streaming Level Save Interface End --------------------
public:
	UInventoryContainerComponent();
	
	/** Init item list, usually used for specific item container game design. */
	UPROPERTY(BlueprintReadWrite, Replicated, EditAnywhere, Category = "Inventory System|Container Component")
	FInventoryItemList ItemList;
	
public:
	
	UFUNCTION(BlueprintPure, Category = "Inventory System|Container Component")
	UInventoryItemInstance* GetItemInstance(int Index) const;

	// Only for editor!!! Used to modify container in editor.
	UFUNCTION(BlueprintCallable, Category = "Inventory System|Container Component")
	void BeginModify();
	
	// Only for editor!!! Used to modify container in editor.
	UFUNCTION(BlueprintCallable, Category = "Inventory System|Container Component")
	void DirtyPackage();
	
	///////////////
	// Save Load //
	///////////////

	FInventoryItemListSaveData GetContainerSaveData() const;
	void LoadSaveData(UPARAM(ref)const FInventoryItemListSaveData& InSaveData);
};
