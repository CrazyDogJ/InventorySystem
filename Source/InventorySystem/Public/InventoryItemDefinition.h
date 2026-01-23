// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemInstance.h"
#include "InventoryUtility.h"
#include "Engine/DataAsset.h"
#include "StructUtils/InstancedStruct.h"
#include "InventoryItemDefinition.generated.h"

class UInventoryItemProcessor;
class UInventoryManageOperation;

/**
 * Definition of the inventory item.
 */
UCLASS(BlueprintType)
class INVENTORYSYSTEM_API UInventoryItemDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
	/** Item id, used to identity or display text key for localization. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Base Info")
	FString ItemID;

	/** Item display name, localization key associate with item id. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Base Info")
	FText ItemDisplayName;

	/** Item display description, localization key associate with item id. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Base Info")
	FText ItemDisplayDescription;
	
	/** Determine what default properties the item has. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(ExcludeBaseStruct), Category = "Storage Info")
	TArray<FInstancedStruct> DefaultFragments;

	/** Item instance type. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Instanced, Category = "Storage Info")
	UInventoryItemInstance* ItemInstance;

	/** Determine how the item actor represent in the game world. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(ExcludeBaseStruct), Category = "Storage Info")
	TInstancedStruct<FItemActorDescBase> ItemActorDesc;

public:
	// Return default item fragment.
	template <typename T>
		const T* GetFragmentPtr() const
	{
		const UScriptStruct* StructType = T::StaticStruct();
		if (const auto Found = TypeLookup.Find(StructType))
		{
			return DefaultFragments[*Found].GetPtr<T>();
		}
		
		return nullptr;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory System")
	UPARAM(DisplayName = "Property")
	FInstancedStruct FindFragmentByClass(const UScriptStruct* StructType, bool& bValid) const;

protected:
	void RebuildLookup();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

	// Accelerate look up.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, AdvancedDisplay, Category = "Storage Info")
	TMap<const UScriptStruct*, int> TypeLookup;
};
