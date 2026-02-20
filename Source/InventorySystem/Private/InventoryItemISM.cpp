// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemISM.h"

#include "ReplicatedFoliageManager.h"
#include "ReplicatedFoliageSubsystem.h"

FInventoryItemEntry UInventoryItemISM::MakeItemEntry() const
{
	FInventoryItemEntry NewEntry = FInventoryItemEntry();
	NewEntry.ItemDefinition = ItemDefinition;
	NewEntry.ItemStack = 1;
	NewEntry.ItemInstance = nullptr;
	return NewEntry;
}

FInventoryItemEntry UInventoryItemISM::GetItemEntry_Implementation()
{
	return MakeItemEntry();
}

bool UInventoryItemISM::IsItemEntryValid_Implementation(const int InstanceIndex)
{
	// Check if instance index is valid.
	return IsValidInstance(InstanceIndex);
}

void UInventoryItemISM::OnEntryPickedUp_Implementation(int Index)
{
	const auto Subsystem = GetWorld()->GetSubsystem<UReplicatedFoliageSubsystem>();

	if (const auto ReplicatedFoliageManager = Subsystem->GetOrSpawnReplicatedFoliageManager())
	{
		ReplicatedFoliageManager->InstanceGetDamaged(this, Index, 100.0f, FVector());
	}
}

void UInventoryItemISM::ReceivePointDamage(int32 InstanceIndex, float Damage, class AController* InstigatedBy,
	FVector HitLocation, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser)
{
	// Ignore parent function.
}

void UInventoryItemISM::ReceiveRadialDamage(const TArray<int32>& Instances, const TArray<float>& Damages,
	class AController* InstigatedBy, FVector Origin, float MaxRadius, const class UDamageType* DamageType,
	AActor* DamageCauser)
{
	// Ignore parent function.
}
