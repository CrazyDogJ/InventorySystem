// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/ItemActor_StaticMesh.h"

AItemActor_StaticMesh::AItemActor_StaticMesh()
{
	PrimaryActorTick.bCanEverTick = true;
	RootStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootStaticMesh"));
	SetRootComponent(RootStaticMeshComponent);
	RootStaticMeshComponent->bApplyImpulseOnDamage = true;
}

void AItemActor_StaticMesh::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other,
	class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal,
	FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	RootStaticMeshComponent->SetSimulatePhysics(true);
}

float AItemActor_StaticMesh::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	if (bReceiveHitAndDamage)
	{
		RootStaticMeshComponent->SetSimulatePhysics(true);
	}
	
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AItemActor_StaticMesh::RegisterWakeEvents(const bool bRegisterOrNot)
{
	bReceiveHitAndDamage = bRegisterOrNot;
	RootStaticMeshComponent->BodyInstance.bNotifyRigidBodyCollision = bReceiveHitAndDamage;
}

void FItemActorDesc_Static::SetupActor(AInventoryItemActor* InItemActor)
{
	if (const auto ItemActor = Cast<AItemActor_StaticMesh>(InItemActor))
	{
		ItemActor->RootStaticMeshComponent->SetStaticMesh(StaticMesh);
		ItemActor->RootStaticMeshComponent->SetMassOverrideInKg(NAME_None, MassOverrideInKg, bShouldOverrideMass);
		ItemActor->RootStaticMeshComponent->SetCollisionObjectType(CollisionChannel);
		ItemActor->RootStaticMeshComponent->SetCollisionResponseToChannels(MeshCollisionResponseContainer);
		ItemActor->RootStaticMeshComponent->SetSimulatePhysics(bSimulatePhysics && !bLockPhysicsOnStart);
		ItemActor->RegisterWakeEvents(bSimulatePhysics && bLockPhysicsOnStart);
	}
}
