// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_InstancedStructQuery.h"

#define LOCTEXT_NAMESPACE "K2Node"



///////////////////////////////////////////////////////////////////////////
// Node Change events interface implementations
///////////////////////////////////////////////////////////////////////////


void UK2Node_InstancedStructQuery::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	AllocateDefaultPins();
	RestoreSplitPins(OldPins);

}

void UK2Node_InstancedStructQuery::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();
	RefreshOutputType();
}

void UK2Node_InstancedStructQuery::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);

	if (Pin && Pin == GetPropertyTypePin())
	{
		RefreshOutputType();
	}
}

void UK2Node_InstancedStructQuery::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	if (Pin == GetPropertyTypePin())
	{
		if (Pin->LinkedTo.Num() == 0)
		{
			RefreshOutputType();
		}
	}
}

void UK2Node_InstancedStructQuery::PostReconstructNode()
{
	Super::PostReconstructNode();
	RefreshOutputType();
}

void UK2Node_InstancedStructQuery::RefreshOutputType()
{

	UEdGraphPin* OutStructPin = GetOutStructPin();
	UEdGraphPin* PropertyTypePin = GetPropertyTypePin();

	if (PropertyTypePin->DefaultObject != OutStructPin->PinType.PinSubCategoryObject)
	{
		if (OutStructPin->SubPins.Num() > 0)
		{
			GetSchema()->RecombinePin(OutStructPin);
		}

		OutStructPin->PinType.PinSubCategoryObject = PropertyTypePin->DefaultObject;
		OutStructPin->PinType.PinCategory = (PropertyTypePin->DefaultObject == nullptr) ? UEdGraphSchema_K2::PC_Wildcard : UEdGraphSchema_K2::PC_Struct;
	}
}




////////////////////////////////////////////////////////////////////////////
//UK2_Node and EDGraph Interface Implementations
////////////////////////////////////////////////////////////////////////////


FString UK2Node_InstancedStructQuery::GetPinMetaData(FName InPinName, FName InKey)
{
	FString MetaData = Super::GetPinMetaData(InPinName, InKey);
	if (MetaData.IsEmpty())
	{ //Filters out the abstract classes from the pin search
		if (InPinName == GetPropertyTypePin()->GetName() && InKey == FBlueprintMetadata::MD_AllowAbstractClasses)
		{
			MetaData = TEXT("false");
		}
	}
	return MetaData;
}

FSlateIcon UK2Node_InstancedStructQuery::GetIconAndTint(FLinearColor& OutColor) const
{
	OutColor = FLinearColor(FColor::Cyan);
	return FSlateIcon(FAppStyle::GetAppStyleSetName(), "Kismet.AllClasses.FunctionIcon");
}


FLinearColor UK2Node_InstancedStructQuery::GetNodeTitleColor() const
{
	return FLinearColor(FColor::Cyan);
}


FText UK2Node_InstancedStructQuery::GetMenuCategory() const
{
  return LOCTEXT("K2Node_InstancedStructQuery", "Inventory System");
}


///////////////////////////////////////////////////////////////////
// PIN GETTERS
///////////////////////////////////////////////////////////////////

UEdGraphPin* UK2Node_InstancedStructQuery::GetTargetPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UK2Node_InstancedStructQuery_PinNames::TargetPinName);
	check(Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_InstancedStructQuery::GetInstancedStructPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UK2Node_InstancedStructQuery_PinNames::InstancedStructPinName);
	check(Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_InstancedStructQuery::GetKeyPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UK2Node_InstancedStructQuery_PinNames::KeyPinName);
	check(Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_InstancedStructQuery::GetPropertyTypePin() const
{
	UEdGraphPin* Pin = FindPinChecked(UK2Node_InstancedStructQuery_PinNames::PropertyTypePinName);
	check(Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_InstancedStructQuery::GetOutStructPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UK2Node_InstancedStructQuery_PinNames::OutStructPinName);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin* UK2Node_InstancedStructQuery::GetValidPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UK2Node_InstancedStructQuery_PinNames::ValidExecPinName);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin* UK2Node_InstancedStructQuery::GetInvalidPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UK2Node_InstancedStructQuery_PinNames::InvalidExecPinName);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}