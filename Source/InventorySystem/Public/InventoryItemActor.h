// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryEntryInterface.h"
#include "StreamingLevelSaveInterface.h"
#include "FastArraySerializers/InventoryListContainer.h"
#include "GameFramework/Actor.h"
#include "InventoryItemActor.generated.h"

class UStreamingLevelSaveComponent;
class UInventoryItemDefinition;

UCLASS()
class INVENTORYSYSTEM_API AInventoryItemActor : public AActor, public IStreamingLevelSaveInterface, public IInventoryEntryInterface
{
	GENERATED_BODY()
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Streaming Level Save Interface Start --------------------
	virtual FInstancedStruct GetSaveData_Implementation() override;
	virtual void LoadSaveData_Implementation(const FInstancedStruct& SaveData) override;
	// Streaming Level Save Interface End --------------------

	// Inventory Entry Interface
	virtual FInventoryItemEntry GetItemEntry_Implementation() override { return ItemEntry; }
	virtual void SetItemEntry_Implementation(FInventoryItemEntry NewEntry) override;
	virtual bool IsItemEntryValid_Implementation(const int InstanceIndex) override { return true; }
	virtual void OnEntryPickedUp_Implementation(int Index) override;
	// Inventory Entry Interface
public:
	AInventoryItemActor();
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_ItemEntry, Category = "Inventory System|Item Actor")
	FInventoryItemEntry ItemEntry;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient)
	UStreamingLevelSaveComponent* LevelSaveComponent;
	
public:
#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, CallInEditor, DisplayName = "Refresh Additional Item Slot Data", Category = "Inventory System|Item Actor")
	void RefreshItemInstance();

	void CheckShouldRefresh();
	void OnItemDefPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);
#endif
	
	void TransToRuntimeActor();

protected:
	UFUNCTION()
	virtual void OnRep_ItemEntry();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|Item Actor")
	void OnItemActorPickedUp();

	virtual void PostTransRuntime() {}
	virtual void NativeOnItemActorPickedUp() { Destroy(); }
};
