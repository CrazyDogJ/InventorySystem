// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InventoryItemAction.generated.h"

class UAbilitySystemComponent;
class UInventoryContainerComponent;

USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FInventoryItemActionContext
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UInventoryContainerComponent> Container = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Index = -1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UInventoryContainerComponent> OtherContainer = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UAbilitySystemComponent> InstigatorAsc = nullptr;
};

UCLASS(Blueprintable, BlueprintType, DefaultToInstanced, EditInlineNew)
class INVENTORYSYSTEM_API UInventoryItemAction : public UObject
{
	GENERATED_BODY()

public:
	UInventoryItemAction() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Inventory")
	void OnActionExecute(const FInventoryItemActionContext& Context);

	virtual void OnActionExecute_Implementation(const FInventoryItemActionContext& Context) {}
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FInventoryItemActionDesc
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText ActionName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Instanced)
	TObjectPtr<UInventoryItemAction> ActionInstance;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bLocalAction = false;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemFragment_ItemActions
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FInventoryItemActionDesc> ItemActions;
};
