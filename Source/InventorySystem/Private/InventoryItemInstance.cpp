// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemInstance.h"

#include "InventoryContainerComponent.h"
#include "InventoryItemActor.h"
#include "InventoryItemDefinition.h"
#include "InventorySystem.h"
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

class UWorld* UInventoryItemInstance::GetWorld() const
{
	if (const auto OuterWorld = GetOuter()->GetWorld())
	{
		return OuterWorld;
	}
	
	return UObject::GetWorld();
}

UInventoryItemInstance* UInventoryItemInstance::NewItemInstance(UObject* Outer, UInventoryItemDefinition* InItemDefinition)
{
	if (!InItemDefinition || !Outer) return nullptr;
	
	auto TemplateInstance = InItemDefinition->ItemInstance;
	// If item def not implement item instance class, use default class.
	if (!TemplateInstance)
	{
		TemplateInstance = NewObject<UInventoryItemInstance>(Outer, StaticClass());
		UE_LOG(LogInventorySystem, Display, TEXT("InventoryItemInstance::NewItemInstance : Item Instance in %s is not valid!"),
			*InItemDefinition->GetName());
	}
	
	const auto NewItemInstance = NewObject<UInventoryItemInstance>(Outer, TemplateInstance->GetClass(), NAME_None, RF_NoFlags, TemplateInstance);
	NewItemInstance->ItemDefinition = InItemDefinition;
	if (const auto CurrentWorld = Outer->GetWorld(); CurrentWorld && CurrentWorld->IsGameWorld())
	{
		NewItemInstance->AddSubObject();
	}
	
	return NewItemInstance;
}

UInventoryContainerComponent* UInventoryItemInstance::TryGetContainerComponent() const
{
	return Cast<UInventoryContainerComponent>(GetOuter());
}

AInventoryItemActor* UInventoryItemInstance::TryGetItemActor() const
{
	return Cast<AInventoryItemActor>(GetOuter());
}

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass());
	if (BPClass != NULL)
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}
	
	DOREPLIFETIME(ThisClass, ItemDefinition);
}

void UInventoryItemInstance::BeginDestroy()
{
	RemoveSubObject();
	
	// Clean up.
	ItemDefinition = nullptr;
	
	UObject::BeginDestroy();
}

void UInventoryItemInstance::Tick(float DeltaTime)
{
	OnTick(DeltaTime);
}

bool UInventoryItemInstance::IsTickable() const
{
	if (this->IsDefaultSubobject())
	{
		return false;
	}

	if (!IsValid(this))
	{
		return false;
	}
	
	if (IsUnreachable())
	{
		return false;
	}
	
	if (const auto World = GetWorld())
	{
		if (World->IsGameWorld())
		{
			return bTickable;
		}
	}
	
	return false;
}

int32 UInventoryItemInstance::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (const auto Comp = TryGetContainerComponent())
	{
		return Comp->GetFunctionCallspace(Function, Stack);
	}
	if (const auto Actor = TryGetItemActor())
	{
		return Actor->GetFunctionCallspace(Function, Stack);
	}
 
	return FunctionCallspace::Local;
}

bool UInventoryItemInstance::CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms,
	FFrame* Stack)
{
	// Get net driver actor
	AActor* NetDriverActor = TryGetItemActor();
	if (!NetDriverActor)
	{
		if (const auto Comp = TryGetContainerComponent())
		{
			NetDriverActor = Comp->GetOwner();
		}
	}
	// Process
	if (NetDriverActor)
	{
		if (UNetDriver* NetDriver = NetDriverActor->GetNetDriver())
		{
			NetDriver->ProcessRemoteFunction(NetDriverActor, Function, Parms, OutParms, Stack, this);
			return true;
		}
	}
	
	return false;
}
