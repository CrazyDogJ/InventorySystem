// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node_InstancedStructQuery.h"
#include "K2Node_GetFragmentByItemDef.generated.h"

UCLASS(BlueprintType, Blueprintable)
class INVENTORYSYSTEMNODES_API UK2Node_GetFragmentByItemDef : public UK2Node_InstancedStructQuery
{
	GENERATED_BODY()

	//~ UEdGraphNode Interface.
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetKeywords() const override;
	virtual void AllocateDefaultPins() override;

	//~ UK2Node Interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
};
