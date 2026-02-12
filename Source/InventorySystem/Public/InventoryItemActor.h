// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StreamingLevelSaveInterface.h"
#include "FastArraySerializers/InventoryListContainer.h"
#include "GameFramework/Actor.h"
#include "InventoryItemActor.generated.h"

class UStreamingLevelSaveComponent;
class UInventoryItemDefinition;

UCLASS()
class INVENTORYSYSTEM_API AInventoryItemActor : public AActor, public IStreamingLevelSaveInterface
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
public:
	AInventoryItemActor();
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_ItemEntry, Category = "Inventory System|Item Actor")
	FInventoryItemEntry ItemEntry;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UStreamingLevelSaveComponent* LevelSaveComponent;
	
public:
#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, CallInEditor, DisplayName = "Refresh Additional Item Slot Data", Category = "Inventory System|Item Actor")
	void RefreshItemInstance();

	void CheckShouldRefresh();
	void OnItemDefPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);
#endif
	// On actor picked up.
	void NotifyItemActorPickedUp();

	void TransToRuntimeActor();

protected:
	UFUNCTION()
	virtual void OnRep_ItemEntry();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|Item Actor")
	void OnItemActorPickedUp();

	virtual void PostTransRuntime() {}
	virtual void NativeOnItemActorPickedUp() { Destroy(); }
};
