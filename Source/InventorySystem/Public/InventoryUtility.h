// Copyright CrazyDog. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryUtility.generated.h"

class AInventoryItemActor;
class UInventoryItemInstance;
class UInventoryItemDefinition;

/** Native Multicast delegate. (for c++ using)*/
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFastArraySerializerEvent, const TArray<int>&);
/** Dynamic Single delegate. (for bp binding using)*/
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnFastArraySerializerStaticEvent, const TArray<int>&, Indices);

/** Actor Desc base struct. */
USTRUCT(BlueprintType)
struct FItemActorDescBase
{
	GENERATED_BODY()
	
	virtual ~FItemActorDescBase() = default;
	
	virtual bool IsDataValid() { return false; }
	virtual TSubclassOf<AInventoryItemActor> GetItemActorClass();
	virtual void SetupActor(AInventoryItemActor* InItemActor) {}
};
