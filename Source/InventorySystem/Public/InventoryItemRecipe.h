// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryListContainer.h"
#include "Engine/DataAsset.h"
#include "InventoryItemRecipe.generated.h"

// TODO : Item recipe is still progressing.

/** Inventory item recipe for crafting system. */
UCLASS(BlueprintType)
class INVENTORYSYSTEM_API UInventoryItemRecipe : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FNameTextStruct BaseInfo;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FItemStackMapping InItems;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FItemStackMapping OutItems;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag RecipeTag;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
