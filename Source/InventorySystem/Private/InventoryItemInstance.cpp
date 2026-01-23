// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemInstance.h"

#include "InventoryItemDefinition.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

UInventoryItemInstance::UInventoryItemInstance()
	: ItemDefinition(nullptr)
{
}

void UInventoryItemInstance::ChangeOuter(UObject* NewOuter)
{
	if (NewOuter != nullptr && GetOuter() != NewOuter)
	{
		RemoveSubObject();
		Rename(nullptr, NewOuter);
		AddSubObject();
	}
}

void UInventoryItemInstance::AddSubObject()
{
	// If outer actor, we add instance to replicated sub-object(Usually the InventoryInfoActor or InventoryItemActor.)
	if (const auto OuterActor = Cast<AActor>(GetOuter()))
	{
		OuterActor->AddReplicatedSubObject(this);
	}
	else if (const auto OuterComp = Cast<UActorComponent>(GetOuter()))
	{
		OuterComp->AddReplicatedSubObject(this);
	}
}

void UInventoryItemInstance::RemoveSubObject()
{
	// If outer actor, we remove instance and destroy.
	if (const auto OuterActor = Cast<AActor>(GetOuter()))
	{
		OuterActor->RemoveReplicatedSubObject(this);
	}
	else if (const auto OuterComp = Cast<UActorComponent>(GetOuter()))
	{
		OuterComp->RemoveReplicatedSubObject(this);
	}
}

void UInventoryItemInstance::GetSaveData(TArray<uint8>& OutSaveData)
{
	FMemoryWriter WriterArchive(OutSaveData, true);
	FObjectAndNameAsStringProxyArchive Ar(WriterArchive, false);
	Serialize(Ar);
}

void UInventoryItemInstance::LoadSaveData(const TArray<uint8>& InSaveData)
{
	if (!InSaveData.IsEmpty())
	{
		FMemoryReader MemoryReader(InSaveData, true);
		FObjectAndNameAsStringProxyArchive Ar(MemoryReader, true);
		Serialize(Ar);
	}
}

UInventoryItemInstance* UInventoryItemInstance::NewItemInstance(UObject* Outer, UInventoryItemDefinition* InItemDefinition)
{
	if (!InItemDefinition || !Outer) return nullptr;
	
	const auto TemplateInstance = InItemDefinition->ItemInstance;
	if (!TemplateInstance) return nullptr;
	
	const auto NewItemInstance = NewObject<UInventoryItemInstance>(Outer, TemplateInstance->GetClass(), NAME_None, RF_NoFlags, TemplateInstance);
	NewItemInstance->ItemDefinition = InItemDefinition;
	if (const auto CurrentWorld = Outer->GetWorld(); CurrentWorld && CurrentWorld->IsGameWorld())
	{
		NewItemInstance->AddSubObject();
	}
	
	return NewItemInstance;
}

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, ItemDefinition);
}

void UInventoryItemInstance::BeginDestroy()
{
	RemoveSubObject();
	
	// Clean up.
	ItemDefinition = nullptr;
	
	UObject::BeginDestroy();
}
