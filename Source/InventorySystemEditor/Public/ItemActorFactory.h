// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryUtility.h"
#include "ActorFactories/ActorFactory.h"
#include "ItemActorFactory.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMEDITOR_API UItemActorFactory : public UActorFactory
{
	GENERATED_BODY()

public:
	UItemActorFactory(const FObjectInitializer& ObjectInitializer);

	FItemActorDescBase* GetItemActorDesc(const FAssetData& AssetData);
	
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	virtual AActor* SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform, const FActorSpawnParameters& InSpawnParams) override;
	virtual FString GetDefaultActorLabel(UObject* Asset) const override;

};
