// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventorySaveGame.h"
#include "StreamingLevelSaveInterface.h"
#include "GameFramework/Actor.h"
#include "InventoryItemActor.generated.h"

struct FInventoryItemActorSaveData;
class UInventoryItemDefinition;

UCLASS()
class INVENTORYSYSTEM_API AInventoryItemActor : public AActor, public IStreamingLevelSaveInterface
{
	GENERATED_BODY()
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	// Streaming Level Save Interface Start --------------------
	virtual FInstancedStruct GetSaveData_Implementation() override;
	virtual void LoadSaveData_Implementation(const FInstancedStruct& SaveData) override;
	// Streaming Level Save Interface End --------------------
public:
	AInventoryItemActor();
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Item Actor")
	UInventoryItemDefinition* ItemDefinition = nullptr;
#endif
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Instanced, ReplicatedUsing = OnRep_ItemInstance, Category = "Inventory System|Item Actor")
	UInventoryItemInstance* ItemInstance;
	
public:
#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, CallInEditor, DisplayName = "Refresh Additional Item Slot Data", Category = "Inventory System|Item Actor")
	void RefreshItemInstance();
#endif
	// On actor picked up.
	void NotifyItemActorPickedUp();

	void TransToRuntimeActor();
	
	void GetSaveData(FInventoryItemEntrySaveData& OutSaveData) const;
	void LoadSaveData(const FInventoryItemEntrySaveData& InSaveData);

protected:
	UFUNCTION()
	void OnRep_ItemInstance();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory System|Item Actor")
	void OnItemActorPickedUp();
	
	virtual void NativeOnItemActorPickedUp() { Destroy(); }
};
