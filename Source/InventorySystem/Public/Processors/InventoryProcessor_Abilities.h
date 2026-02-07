// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "InventoryUtility.h"
#include "UObject/Object.h"
#include "InventoryProcessor_Abilities.generated.h"

class UAttributeSet;
class UGameplayAbility;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FAbilitySet_GameplayAbility
{
	GENERATED_BODY()
	
	// Gameplay ability to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> Ability = nullptr;

	// Level of ability to grant.
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	// Tag used to process input for the ability.
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FAbilitySet_GameplayEffect
{
	GENERATED_BODY()
	
	// Gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	// Level of gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FAbilitySet_AttributeSet
{
	GENERATED_BODY()
	
	// Gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet;

};

USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(const TObjectPtr<UAttributeSet>& Set);

	void TakeFromAbilitySystem(const TObjectPtr<UAbilitySystemComponent>& ASC);

protected:
	// Handles to the granted abilities.
	UPROPERTY(NotReplicated)
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// Handles to the granted gameplay effects.
	UPROPERTY(NotReplicated)
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	// Pointers to the granted attribute sets
	UPROPERTY(NotReplicated)
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};

/** Item abilities fragment. */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FItemFragment_Abilities
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FAbilitySet_GameplayAbility> Abilities;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FAbilitySet_GameplayEffect> Effects;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FAbilitySet_AttributeSet> AttributeSets;

	// Grants the ability set to the specified ability system component.
	// The returned handles can be used later to take away anything that was granted.
	void GiveToAbilitySystem(const TObjectPtr<UAbilitySystemComponent>& ASC, FAbilitySet_GrantedHandles& OutGrantedHandles, const TObjectPtr<
							 UObject>& SourceObject = nullptr) const;
};

UINTERFACE(MinimalAPI)
class UAbilityItem : public UInterface
{
	GENERATED_BODY()
};

class INVENTORYSYSTEM_API IAbilityItem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory System|Abilities")
	FAbilitySet_GrantedHandles GetAbilitySetGrantedHandles() const;
	virtual FAbilitySet_GrantedHandles GetAbilitySetGrantedHandles_Implementation() const { return FAbilitySet_GrantedHandles(); }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory System|Abilities")
	void SetAbilitySetGrantedHandles(FAbilitySet_GrantedHandles InHandles);
	virtual void SetAbilitySetGrantedHandles_Implementation(FAbilitySet_GrantedHandles InHandles) const {}
};

/** Item abilities functions for blueprint user. */
UCLASS()
class UItemProcessor_Abilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool IsAbilityItem(const UInventoryItemInstance* InItemInstance);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory System|Abilities", BlueprintAuthorityOnly)
	static bool GiveToAbilitySystem(UInventoryItemInstance* InItemInstance, UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, Category = "Inventory System|Abilities", BlueprintAuthorityOnly)
	static bool TakeFromAbilitySystem(UInventoryItemInstance* InItemInstance, UAbilitySystemComponent* ASC);
};