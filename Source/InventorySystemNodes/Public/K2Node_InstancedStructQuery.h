// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_InstancedStructQuery.generated.h"

namespace UK2Node_InstancedStructQuery_PinNames
{
	static FName TargetPinName = "Target";
	static FName InstancedStructPinName = "InstancedStruct";
	static FName KeyPinName = "Key";
	static FName PropertyTypePinName = "PropertyType";
	static FName OutStructPinName = "Property";
	static FName ValidExecPinName = "Valid";
	static FName InvalidExecPinName = "Invalid";
};

UCLASS(BlueprintType, Blueprintable)
class INVENTORYSYSTEMNODES_API UK2Node_InstancedStructQuery : public UK2Node
{
	GENERATED_BODY()

protected:

	//~ UEdGraphNode Interface.
	virtual void PinConnectionListChanged(UEdGraphPin* Pin);
	virtual void PostPlacedNewNode() override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual FString GetPinMetaData(FName InPinName, FName InKey) override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual FLinearColor GetNodeTitleColor() const override;


	//~ UK2Node Interface
	virtual FText GetMenuCategory() const override;
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	virtual bool IsNodeSafeToIgnore() const override { return true; }
	virtual bool IsNodePure() const override { return false; }
	virtual void PostReconstructNode() override;

	//~ Getters
	UEdGraphPin* GetTargetPin() const;
	UEdGraphPin* GetInstancedStructPin() const;
	UEdGraphPin* GetKeyPin() const;
	UEdGraphPin* GetPropertyTypePin() const;
	UEdGraphPin* GetOutStructPin() const;
	UEdGraphPin* GetValidPin() const;
	UEdGraphPin* GetInvalidPin() const;

	//~ Helper Functions
	void RefreshOutputType();
};
