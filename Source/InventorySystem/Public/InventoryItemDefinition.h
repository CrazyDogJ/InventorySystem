// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedFragmentContainer.h"
#include "InventoryUtility.h"
#include "NameTextStruct.h"
#include "Engine/DataAsset.h"
#include "StructUtils/InstancedStruct.h"
#include "InventoryItemDefinition.generated.h"

class UInventorySlotDataBase;

#if WITH_EDITOR
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemDefinitionPropertyChanged, const FPropertyChangedEvent& PropertyChangedEvent)
#endif

/**
 * Definition of the inventory item.
 */
UCLASS(BlueprintType)
class INVENTORYSYSTEM_API UInventoryItemDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UInventoryItemDefinition();
	
	/** Item base info struct, from InstancedStructFragment plugin. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FNameTextStruct ItemBaseInfo{"",FText(), FText(), "InventorySystem"};

	/** Item default fragments, from InstancedStructFragment plugin. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FInstancedFragmentContainer ItemDefaultFragments;

	/** Item slot data definition. Always CDO for logic calculation. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UInventorySlotDataBase> ItemSlotDataDefinition;

	/** Item tag for filter. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag ItemGameplayTag;

	/** Determine how the item actor represent in the game world. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(ExcludeBaseStruct))
	TInstancedStruct<FItemActorDescBase> ItemActorDesc;

public:
	// Return default item fragment.
	template <typename T>
		const T* GetFragmentPtr() const
	{
		return ItemDefaultFragments.GetFragmentPtr<T>();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UInventorySlotDataBase* GetSlotDataDefinitionCDO() const;
	
#if WITH_EDITORONLY_DATA
	FOnItemDefinitionPropertyChanged PropertyChangedDelegate;
#endif

protected:
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
