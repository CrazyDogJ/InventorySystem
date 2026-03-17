// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemDefinition.h"
#include "StreamingLevelSaveInterface.h"
#include "Components/ActorComponent.h"
#include "FastArraySerializers/InventoryListContainer.h"
#include "InventoryContainerComponent.generated.h"

class UStreamingLevelSaveComponent;
class AInventoryInfoActor;
struct FInventoryItemListSaveData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemPickedUp, const UInventoryItemDefinition*, ItemDef, int, Amount);

/**
 * An inventory component that attach to visible actors.
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
	UPROPERTY(BlueprintReadWrite, Replicated, VisibleAnywhere, Transient, Category = "Inventory System|Container Component")
	FInventoryItemList ItemList;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Container Component")
	bool bShouldInit;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Container Component", meta = (EditCondition = "bShouldInit"))
	FInventoryItemList InitItemList;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Container Component")
	bool bAutoStreamingSave = false;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Inventory System|Container Component")
	UStreamingLevelSaveComponent* StreamingLevelSaveComponent;

	/** Visual multicast event for item picked. */
	UFUNCTION(NetMulticast, Unreliable)
	void OnItemPickupUpMulticast(const UInventoryItemDefinition* ItemDef, const int Amount);
	
	UPROPERTY(BlueprintAssignable)
	FOnItemPickedUp OnItemPickedUpEvent;
};
