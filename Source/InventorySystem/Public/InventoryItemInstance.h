// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InventoryItemInstance.generated.h"

enum class EStructUtilsResult : uint8;
class UInventoryItemDefinition;

/**
 * Inventory item instance
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class INVENTORYSYSTEM_API UInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UInventoryItemInstance();
	static UInventoryItemInstance* NewItemInstance(UObject* Outer, UInventoryItemDefinition* InItemDefinition);
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated, Category = "Inventory System|Item Instance")
	UInventoryItemDefinition* ItemDefinition;

public:
	// Sub-object Start ----------------------------------------
	/** Change outer object and replicated sub-object. */
	void ChangeOuter(UObject* NewOuter);
	/** Add sub-object in current outer. */
	void AddSubObject();
	/** Remove sub-object in current outer. */
	void RemoveSubObject();
	// Sub-object End ----------------------------------------

	void GetSaveData(TArray<uint8>& OutSaveData);
	void LoadSaveData(const TArray<uint8>& InSaveData);

protected:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginDestroy() override;
};
