// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryEntryInterface.h"
#include "InventoryListContainer.h"
#include "GameFramework/Actor.h"
#include "InventoryItemActor.generated.h"

class UInventoryItemDefinition;

UCLASS()
class INVENTORYSYSTEM_API AInventoryItemActor : public AActor, public IInventoryEntryInterface
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;

	// Inventory Entry Interface
	virtual FInventoryItemEntry GetItemEntry_Implementation() override { return ItemEntry; }
	virtual void SetItemEntry_Implementation(FInventoryItemEntry NewEntry) override;
	virtual bool IsItemEntryValid_Implementation(const int InstanceIndex) override { return true; }
	virtual void OnEntryPickedUp_Implementation(int Index) override;
	// Inventory Entry Interface
public:
	AInventoryItemActor();
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item Actor")
	bool bShouldTickItem = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, ReplicatedUsing = OnRep_ItemEntry, Category = "Item Actor")
	FInventoryItemEntry ItemEntry;
	
	UFUNCTION(BlueprintCallable)
	void PostLoadItemEntry();
public:
#if WITH_EDITOR
	// 修复：重新构造或更换物品定义时解除旧资产上的绑定。
	TWeakObjectPtr<UInventoryItemDefinition> ObservedDefinition;
	UFUNCTION(BlueprintCallable, CallInEditor, DisplayName = "Refresh Additional Item Slot Data", Category = "Item Actor")
	void RefreshItemInstance();

	void CheckShouldRefresh();
	void OnItemDefPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);

	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
	void TransToRuntimeActor();

protected:
	UFUNCTION()
	virtual void OnRep_ItemEntry();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Item Actor")
	void OnItemActorPickedUp();

	virtual void PostTransRuntime() {}
	virtual void NativeOnItemActorPickedUp() { Destroy(); }
};
