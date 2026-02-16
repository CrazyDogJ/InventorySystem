// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/ItemActor_StaticMesh.h"

#include "InventoryItemDefinition.h"
#include "StreamingLevelSaveLibrary.h"

AItemActor_StaticMesh::AItemActor_StaticMesh()
{
	PrimaryActorTick.bCanEverTick = true;
	RootStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootStaticMesh"));
	SetRootComponent(RootStaticMeshComponent);
	RootStaticMeshComponent->bApplyImpulseOnDamage = true;
	RootStaticMeshComponent->SetIsReplicated(true);
}

void AItemActor_StaticMesh::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other,
	class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal,
	FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	Interact();
}

float AItemActor_StaticMesh::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	if (bReceiveHitAndDamage)
	{
		Interact();
	}
	
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AItemActor_StaticMesh::PostTransRuntime()
{
	Interact();
}

void AItemActor_StaticMesh::OnRep_ItemEntry()
{
	Super::OnRep_ItemEntry();

	if (ItemEntry.ItemDefinition && ItemEntry.ItemDefinition->ItemActorDesc.IsValid())
	{
		if (const auto Desc = ItemEntry.ItemDefinition->ItemActorDesc.GetPtr<FItemActorDesc_Static>())
		{
			// These functions should only be called by authority.
			if (HasAuthority())
			{
				RootStaticMeshComponent->SetStaticMesh(Desc->StaticMesh);
			}
			
			RootStaticMeshComponent->SetMassOverrideInKg(NAME_None, Desc->MassOverrideInKg, Desc->bShouldOverrideMass);
			RootStaticMeshComponent->SetCollisionObjectType(Desc->CollisionChannel);
			RootStaticMeshComponent->SetCollisionResponseToChannels(Desc->MeshCollisionResponseContainer);
			RootStaticMeshComponent->SetSimulatePhysics(Desc->bSimulatePhysics && !Desc->bLockPhysicsOnStart);
			RegisterWakeEvents(Desc->bSimulatePhysics && Desc->bLockPhysicsOnStart);
		}
	}
}

void AItemActor_StaticMesh::RegisterWakeEvents(const bool bRegisterOrNot)
{
	bReceiveHitAndDamage = bRegisterOrNot;
	RootStaticMeshComponent->BodyInstance.SetInstanceNotifyRBCollision(bReceiveHitAndDamage);
}

void AItemActor_StaticMesh::Interact()
{
	if (HasAuthority())
	{
		if (!UStreamingLevelSaveLibrary::IsRuntimeObject(this))
		{
			TransToRuntimeActor();
		}
		else
		{
			RootStaticMeshComponent->SetSimulatePhysics(true);
		}
	}
}
