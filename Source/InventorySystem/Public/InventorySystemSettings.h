// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/ItemActor_StaticMesh.h"
#include "Processors/InventoryProcessor_Abilities.h"
#include "UObject/Object.h"
#include "UObject/SoftObjectPtr.h"
#include "InventorySystemSettings.generated.h"

class UInventoryUserWidget;

UCLASS(Config = InventorySystem, DefaultConfig)
class INVENTORYSYSTEM_API UInventorySystemSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Inventory Interfaces")
	TMap<TSoftClassPtr<UInterface>, TSoftClassPtr<UInventoryUserWidget>> UserWidgets;

	UPROPERTY(Config, EditAnywhere, Category = "Inventory Interfaces")
	TMap<FString, TSoftClassPtr<UInventoryUserWidget>> UserWidgets_Cpp;
	
	UPROPERTY(Config, EditAnywhere, Category = "Item Actors")
	TSoftClassPtr<AItemActor_StaticMesh> DefaultStaticMeshItemActorClass;
	
	UPROPERTY(Config, EditAnywhere, Category = "Abilities")
	TSoftClassPtr<UInventoryAbilitySetup> DefaultInventoryAbilitySetup;
	
	TMap<TSoftClassPtr<UInterface>, TSoftClassPtr<UInventoryUserWidget>> GetAllUserWidgets();
	TSubclassOf<UInventoryUserWidget> GetUserWidgetClass(TSubclassOf<UInterface> InInterface);
	TSubclassOf<AItemActor_StaticMesh> GetStaticMeshItemActorClass() const;
	TSubclassOf<UInventoryAbilitySetup> GetDefaultInventoryAbilitySetup() const;
};
