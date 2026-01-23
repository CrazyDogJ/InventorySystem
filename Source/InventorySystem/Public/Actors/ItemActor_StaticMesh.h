// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemActor.h"
#include "InventoryUtility.h"
#include "ItemActor_StaticMesh.generated.h"

/**
 * Item actor that contain static mesh.
 * Use damage to activate physics on this item actor.
 */
UCLASS()
class INVENTORYSYSTEM_API AItemActor_StaticMesh : public AInventoryItemActor
{
	GENERATED_BODY()

public:
	AItemActor_StaticMesh();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Inventory System|Item Actor")
	UStaticMeshComponent* RootStaticMeshComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Inventory System|Item Actor")
	bool bReceiveHitAndDamage = false;
	
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	void RegisterWakeEvents(const bool bRegisterOrNot);
};

/** Default static mesh item actor. */
USTRUCT(BlueprintType)
struct FItemActorDesc_Static : public FItemActorDescBase
{
	GENERATED_BODY()

	/** Item actor class override. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<AItemActor_StaticMesh> ItemActorClassOverride = AItemActor_StaticMesh::StaticClass();
	
	/** Item mesh. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMesh* StaticMesh = nullptr;

	/** Should item simulate physics. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Physics")
	bool bSimulatePhysics = true;

	/** Will lock physics on start. Usually used in overlapping placement item. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Physics", meta = (EditCondition = bSimulatePhysics))
	bool bLockPhysicsOnStart = true;

	/** Should override the item mass. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mass")
	bool bShouldOverrideMass = false;

	/** Item mass. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mass", meta = (EditCondition = bShouldOverrideMass, Units = "kg"))
	float MassOverrideInKg = 1.0f;

	/** Item object channel type. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Collision")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Visibility;

	/** Item object response container. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Collision")
	FCollisionResponseContainer MeshCollisionResponseContainer;

	virtual TSubclassOf<AInventoryItemActor> GetItemActorClass() override { return ItemActorClassOverride; }
	virtual bool IsDataValid() override { return ItemActorClassOverride && StaticMesh != nullptr; }
	virtual void SetupActor(AInventoryItemActor* InItemActor) override;
};