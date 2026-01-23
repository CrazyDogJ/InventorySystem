// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_GetFragmentByItemDef.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "InventoryItemDefinition.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_CallFunction.h"
#include "K2Node_TemporaryVariable.h"
#include "KismetCompiler.h"
#include "Kismet/BlueprintInstancedStructLibrary.h"

#define LOCTEXT_NAMESPACE "K2Node"


void UK2Node_GetFragmentByItemDef::AllocateDefaultPins()
{

	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UInventoryItemDefinition::StaticClass(), UK2Node_InstancedStructQuery_PinNames::TargetPinName);
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UScriptStruct::StaticClass(), UK2Node_InstancedStructQuery_PinNames::PropertyTypePinName);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UK2Node_InstancedStructQuery_PinNames::ValidExecPinName);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UK2Node_InstancedStructQuery_PinNames::InvalidExecPinName);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, UK2Node_InstancedStructQuery_PinNames::OutStructPinName);

	Super::AllocateDefaultPins();
	
}

void UK2Node_GetFragmentByItemDef::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{

	Super::ExpandNode(CompilerContext, SourceGraph);

	UFunction* GetItemPropertyFunc = UInventoryItemDefinition::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UInventoryItemDefinition, FindFragmentByClass));
	UFunction* GetInstStructValueFunc = UBlueprintInstancedStructLibrary::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UBlueprintInstancedStructLibrary, GetInstancedStructValue));

	if (!GetItemPropertyFunc || !GetInstStructValueFunc)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("InvalidClass", "The FindFragmentByClass or GetInstancedStructValue functions have not been found. Check ExpandNode in K2Node_GetFragmentByItemDef.cpp").ToString(), this);
		return;
	}

	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
	bool bIsErrorFree = true;


	const FEdGraphPinType& PinType = GetOutStructPin()->PinType;
	UK2Node_TemporaryVariable* TempVarOutput = CompilerContext.SpawnInternalVariable(
		this, PinType.PinCategory, PinType.PinSubCategory, PinType.PinSubCategoryObject.Get(), PinType.ContainerType, PinType.PinValueType);
	
	UK2Node_AssignmentStatement* AssignNode = CompilerContext.SpawnIntermediateNode<UK2Node_AssignmentStatement>(this, SourceGraph);
	UK2Node_CallFunction* const GetPropertyTypeNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	UK2Node_CallFunction* const GetStructValueNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);

	GetPropertyTypeNode->SetFromFunction(GetItemPropertyFunc);
	GetStructValueNode->SetFromFunction(GetInstStructValueFunc);

	GetPropertyTypeNode->AllocateDefaultPins();
	GetStructValueNode->AllocateDefaultPins();
	AssignNode->AllocateDefaultPins();

	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(GetPropertyTypeNode, this);
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(GetStructValueNode, this);
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(AssignNode, this);

	//Connect input pins to the GetPropertyTypeNode
	//----------------------------------------------------------------------------------------
	UEdGraphPin* TargetPin = Schema->FindSelfPin(*GetPropertyTypeNode, EGPD_Input);
	UEdGraphPin* InterExecLeftPin = GetPropertyTypeNode->GetThenPin();
	UEdGraphPin* PropertyTypePin = GetPropertyTypeNode->FindPinChecked(TEXT("StructType"));
	UEdGraphPin* InterStructLeftPin = GetPropertyTypeNode->GetReturnValuePin();

	bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*GetTargetPin(), *TargetPin).CanSafeConnect();
	bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*GetPropertyTypePin(), *PropertyTypePin).CanSafeConnect();
	//----------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------
	

	//Connect input Target Pin and the output GetPropertyTypeNode pins to the GetStructValueNode pin
	//----------------------------------------------------------------------------------------
	UEdGraphPin* InterExecRightPin = GetStructValueNode->GetExecPin();
	UEdGraphPin* ValidPin = GetStructValueNode->FindPinChecked(TEXT("Valid"));
	UEdGraphPin* InvalidPin = GetStructValueNode->FindPinChecked(TEXT("NotValid"));
	UEdGraphPin* OutStructPin = GetStructValueNode->FindPinChecked(TEXT("Value"));
	UEdGraphPin* InterStructRightPin = GetStructValueNode->FindPinChecked(TEXT("InstancedStruct"));

	bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *InterExecRightPin).CanSafeConnect();
	bIsErrorFree &= Schema->TryCreateConnection(InterStructLeftPin, InterStructRightPin);
	//----------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------


	//Connect the output of GetStructValueNode to the input of Assign node. The Invalid pin directly goes to Invalid pin
	// but the valid pin goes through the Assign node. The temporary variable is assigned a value, and is connected to the
	// node's output struct. The order that the connections are made is critical. First the variable needs to connect to the output,
	// then the variable should be connected to the Variable pin of Assign so the variable type is updated, and only then the Value pin
	// of Assign can be connected to OutStructPin of the GetStructValueNode.
	//----------------------------------------------------------------------------------------
	bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*GetOutStructPin(), *TempVarOutput->GetVariablePin()).CanSafeConnect();
	bIsErrorFree &= Schema->TryCreateConnection(AssignNode->GetVariablePin(), TempVarOutput->GetVariablePin());
	bIsErrorFree &= Schema->TryCreateConnection(AssignNode->GetValuePin(), OutStructPin);

	bIsErrorFree &= Schema->TryCreateConnection(ValidPin, AssignNode->GetExecPin());
	bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*GetInvalidPin(), *InvalidPin).CanSafeConnect();
	bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*GetValidPin(), *AssignNode->GetThenPin()).CanSafeConnect();
	//----------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------
	

	if (!bIsErrorFree)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("InternalConnectionError", "GetFragmentByItemDef: Internal connection error. @@").ToString(), this);
	}
	BreakAllNodeLinks();


}


////////////////////////////////////////////////////////////////////////////
//UK2_Node and EDGraph Interface Implementations
////////////////////////////////////////////////////////////////////////////

void UK2Node_GetFragmentByItemDef::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	Super::GetMenuActions(ActionRegistrar);
	UClass* Action = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(Action))
	{
		UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(Action, Spawner);
	}
}

FText UK2Node_GetFragmentByItemDef::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
  return LOCTEXT("UK2Node_GetFragmentByItemDef_NodeTitle", "Get Item Fragment By Item Definition");
}

FText UK2Node_GetFragmentByItemDef::GetTooltipText() const
{
  return LOCTEXT("UK2Node_GetFragmentByItemDef_TooltipText", "Outputs the item fragment based on the item definition and selected struct type");
}

FText UK2Node_GetFragmentByItemDef::GetKeywords() const
{
  return LOCTEXT("GetItemFragmentByItemDefinition", "Get Item Fragment By Item Definition");
}