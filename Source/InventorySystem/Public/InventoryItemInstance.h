// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InventoryItemInstance.generated.h"

class AInventoryItemActor;
class UInventoryContainerComponent;
enum class EStructUtilsResult : uint8;
class UInventoryItemDefinition;

/**
 * Inventory item instance
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class INVENTORYSYSTEM_API UInventoryItemInstance : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UInventoryItemInstance();
	static UInventoryItemInstance* NewItemInstance(UObject* Outer, UInventoryItemDefinition* InItemDefinition);
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated, Category = "Inventory System|Item Instance")
	UInventoryItemDefinition* ItemDefinition;

	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Instance")
	UInventoryContainerComponent* TryGetContainerComponent() const;

	UFUNCTION(BlueprintPure, Category = "Inventory System|Item Instance")
	AInventoryItemActor* TryGetItemActor() const;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory System|Item Instance")
	bool bTickable = false;

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Tick")
	void OnTick(float DeltaTime);

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
	virtual class UWorld* GetWorld() const override;
#if WITH_EDITOR
	virtual bool ImplementsGetWorld() const override { return true; }
#endif
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginDestroy() override;

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UInventoryItemInstance, STATGROUP_Tickables); }
	
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;
};
