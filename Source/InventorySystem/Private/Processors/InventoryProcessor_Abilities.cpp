// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/InventoryProcessor_Abilities.h"

#include "AbilitySystemComponent.h"
#include "InventoryItemDefinition.h"
#include "InventoryItemInstance.h"

void FAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FAbilitySet_GrantedHandles::AddAttributeSet(const TObjectPtr<UAttributeSet>& Set)
{
	GrantedAttributeSets.Add(Set);
}

void FAbilitySet_GrantedHandles::TakeFromAbilitySystem(const TObjectPtr<UAbilitySystemComponent>& ASC)
{
	if (!ASC)
	{
		return;
	}

	if (!ASC->IsOwnerActorAuthoritative())
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			ASC->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}
	}

	for (UAttributeSet* Set : GrantedAttributeSets)
	{
		ASC->RemoveSpawnedAttribute(Set);
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}

void FItemFragment_Abilities::GiveToAbilitySystem(const TObjectPtr<UAbilitySystemComponent>& ASC,
                                                  FAbilitySet_GrantedHandles& OutGrantedHandles, const TObjectPtr<UObject>& SourceObject) const
{
	if (!ASC)
    {
        return;
    }

    if (!ASC->IsOwnerActorAuthoritative())
    {
        return;
    }

    // GA
    for (int32 AbilityIndex = 0; AbilityIndex < Abilities.Num(); ++AbilityIndex)
    {
        const FAbilitySet_GameplayAbility& AbilityToGrant = Abilities[AbilityIndex];

        if (!IsValid(AbilityToGrant.Ability))
        {
            //UE_LOG(LogInventorySystem, Error, TEXT("Abilities[%d] on ability set [%s] is not valid."), AbilityIndex, *GetNameSafe(this));
            continue;
        }

        UGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UGameplayAbility>();

        FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
        AbilitySpec.SourceObject = SourceObject;
        AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);
        
        const FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(AbilitySpec);
    	OutGrantedHandles.AddAbilitySpecHandle(AbilitySpecHandle);
    }

    // GE
    for (int32 EffectIndex = 0; EffectIndex < Effects.Num(); ++EffectIndex)
    {
        const FAbilitySet_GameplayEffect& EffectToGrant = Effects[EffectIndex];

        if (!IsValid(EffectToGrant.GameplayEffect))
        {
            //UE_LOG(LogInventorySystem, Error, TEXT("Effects[%d] on ability set [%s] is not valid"), EffectIndex, *GetNameSafe(this));
            continue;
        }

        const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
        const FActiveGameplayEffectHandle GameplayEffectHandle = ASC->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, ASC->MakeEffectContext());
    	OutGrantedHandles.AddGameplayEffectHandle(GameplayEffectHandle);
    }

    // Grant the attribute sets.
    for (int32 SetIndex = 0; SetIndex < AttributeSets.Num(); ++SetIndex)
    {
        const FAbilitySet_AttributeSet& SetToGrant = AttributeSets[SetIndex];

        if (!IsValid(SetToGrant.AttributeSet))
        {
            //UE_LOG(LogInventorySystem, Error, TEXT("AttributeSets[%d] on ability set [%s] is not valid"), SetIndex, *GetNameSafe(this));
            continue;
        }

        UAttributeSet* NewSet = NewObject<UAttributeSet>(ASC->GetOwner(), SetToGrant.AttributeSet);
        ASC->AddAttributeSetSubobject(NewSet);

    	OutGrantedHandles.AddAttributeSet(NewSet);
    }
}

bool UItemProcessor_Abilities::IsAbilityItem(const UInventoryItemInstance* InItemInstance)
{
	if (InItemInstance)
	{
		return InItemInstance->GetClass()->ImplementsInterface(UAbilityItem::StaticClass());
	}

	return false;
}

bool UItemProcessor_Abilities::GiveToAbilitySystem(UInventoryItemInstance* InItemInstance, UAbilitySystemComponent* ASC)
{
	if (!InItemInstance || !ASC)
	{
		return false;
	}

	if (IsAbilityItem(InItemInstance) && InItemInstance->ItemDefinition)
	{
		if (const auto AbilitiesSet = InItemInstance->ItemDefinition->GetFragmentPtr<FItemFragment_Abilities>())
		{
			FAbilitySet_GrantedHandles OutGrantedHandles;
			AbilitiesSet->GiveToAbilitySystem(ASC, OutGrantedHandles, InItemInstance);
			IAbilityItem::Execute_SetAbilitySetGrantedHandles(InItemInstance, OutGrantedHandles);
			return true;
		}
	}

	return false;
}

bool UItemProcessor_Abilities::TakeFromAbilitySystem(UInventoryItemInstance* InItemInstance,
	UAbilitySystemComponent* ASC)
{
	if (!InItemInstance || !ASC)
	{
		return false;
	}

	if (IsAbilityItem(InItemInstance))
	{
		auto GrantedHandles = IAbilityItem::Execute_GetAbilitySetGrantedHandles(InItemInstance);
		GrantedHandles.TakeFromAbilitySystem(ASC);
		IAbilityItem::Execute_SetAbilitySetGrantedHandles(InItemInstance, FAbilitySet_GrantedHandles());
		return true;
	}
	
	return false;
}
