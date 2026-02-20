// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryEntryInterface.h"
#include "ReplicatedFoliageISM.h"
#include "InventoryItemISM.generated.h"

class UInventoryItemDefinition;

UCLASS()
class INVENTORYSYSTEM_API UInventoryItemISM : public UReplicatedFoliageISM, public IInventoryEntryInterface
{
	GENERATED_BODY()

public:
	// Item definition that this ism referencing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInventoryItemDefinition* ItemDefinition;

	FInventoryItemEntry MakeItemEntry() const;
	
	// InventoryEntryInterface
	virtual FInventoryItemEntry GetItemEntry_Implementation() override;
	virtual void SetItemEntry_Implementation(FInventoryItemEntry NewEntry) override {}
	virtual bool IsItemEntryValid_Implementation(const int InstanceIndex) override;
	virtual void OnEntryPickedUp_Implementation(int Index) override;
	// InventoryEntryInterface

	// Ignore parent damage function.
	virtual void ReceivePointDamage(int32 InstanceIndex, float Damage, class AController* InstigatedBy, FVector HitLocation, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser) override;
	virtual void ReceiveRadialDamage(const TArray<int32>& Instances, const TArray<float>& Damages, class AController* InstigatedBy, FVector Origin, float MaxRadius, const class UDamageType* DamageType, AActor* DamageCauser) override;
};
