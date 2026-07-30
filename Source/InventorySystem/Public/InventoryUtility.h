// Copyright CrazyDog. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryUtility.generated.h"

class AInventoryItemActor;
class UInventoryItemInstance;
class UInventoryItemDefinition;

/** Native Multicast delegate. (for c++ using)*/
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFastArraySerializerEvent, const TArray<int>&);
/** Dynamic Single delegate. (for bp binding using)*/
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnFastArraySerializerStaticEvent, const TArray<int>&, Indices);
/** ItemStackMapping change event. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FItemStackMappingEvent, const TSoftObjectPtr<UInventoryItemDefinition>&, const int)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FItemStackMappingStaticEvent, const TSoftObjectPtr<UInventoryItemDefinition>&, ItemDef, const int, Amount);

/** Actor Desc base struct. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemActorDescBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Scale = FVector::OneVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bCanDrop = true;
	
	virtual ~FItemActorDescBase() = default;
	
	virtual bool IsDataValid() const { return false; }
	virtual TSubclassOf<AInventoryItemActor> GetItemActorClass() const;
};

/** Item slot entry filter. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemSlotFilter
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<TSoftObjectPtr<UInventoryItemDefinition>> AllowedItems;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer AllowedItemTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bOnlyOne = false;

	bool IsItemAllowed(const UInventoryItemDefinition* Item) const;
	bool IsItemAllowed(const TSoftObjectPtr<UInventoryItemDefinition>& Item) const;
};

/** Item stack count mapping with soft object ptr.
 * Usually used in checking inventory exchange. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemStackMapping
{
	GENERATED_BODY()

	FItemStackMapping() {}
	explicit FItemStackMapping(const TMap<TSoftObjectPtr<UInventoryItemDefinition>, int>& InItems) : Items(InItems) {}
	explicit FItemStackMapping(TMap<TObjectPtr<UInventoryItemDefinition>, int> InItems);
	
	/** Soft object ptr of item definition with stack count mapping. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<TSoftObjectPtr<UInventoryItemDefinition>, int> Items;

	void ChangeItemAmount(const TSoftObjectPtr<UInventoryItemDefinition>& InItemDefinition, int DeltaAmount);
	FItemStackMappingEvent ItemStackMappingEvent;
	
	bool operator==(const FItemStackMapping& InMappings) const
	{
		if (InMappings.Items.Num() != Items.Num())
		{
			return false;
		}
		
		for (const auto Pair : InMappings.Items)
		{
			if (const auto Found = Items.Find(Pair.Key))
			{
				if (*Found != Pair.Value)
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		
		return true;
	}
};