// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemAction.h"
#include "InventoryItemDefinition.h"
#include "Components/ActorComponent.h"
#include "InventoryListContainer.h"
#include "InventoryContainerComponent.generated.h"

class UInventoryItemAction;
struct FInventoryItemListSaveData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemPickedUp, const UInventoryItemDefinition*, ItemDef, int, Amount);

/**
 * An inventory component that attach to visible actors.
 */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent), Blueprintable)
class INVENTORYSYSTEM_API UInventoryContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Virtual function override
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
public:
	UInventoryContainerComponent();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory System|Container Component")
	bool bShouldTickItem = false;
	
	/** Init item list, usually used for specific item container game design. */
	UPROPERTY(BlueprintReadWrite, Replicated, VisibleAnywhere, Transient, Category = "Inventory System|Container Component")
	FInventoryItemList ItemList;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Container Component")
	bool bShouldInit;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory System|Container Component", meta = (EditCondition = "bShouldInit"))
	FInventoryItemList InitItemList;

	/** Visual multicast event for item picked. */
	UFUNCTION(Client, Unreliable)
	void OnItemPickupUpClient(const UInventoryItemDefinition* ItemDef, const int Amount);

	// Usually used in refreshable item container.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void ResetInventoryList();
	
	UPROPERTY(BlueprintAssignable)
	FOnItemPickedUp OnItemPickedUpEvent;

	// Item Actions
	UFUNCTION(BlueprintCallable)
	void ExecuteItemAction(const int Index, const int ActionIndex, UInventoryContainerComponent* OtherContainer = nullptr);

	// Item Actions Server
	UFUNCTION(Server, Reliable)
	void ExecuteItemAction_Server(UInventoryItemAction* ItemAction, const FInventoryItemActionContext& Context);
};
