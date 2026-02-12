// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemEntryPropertyTypeCustomization.h"

#include "ContentBrowserModule.h"
#include "DetailWidgetRow.h"
#include "IContentBrowserSingleton.h"
#include "IDetailChildrenBuilder.h"
#include "InventoryItemDefinition.h"
#include "Processors/InventoryProcessor_Stackable.h"

#define LOCTEXT_NAMESPACE "FItemEntryPropertyTypeCustomization"

TSharedRef<IPropertyTypeCustomization> FItemEntryPropertyTypeCustomization::MakeInstance()
{
	return MakeShared<FItemEntryPropertyTypeCustomization>();
}

FReply FItemEntryPropertyTypeCustomization::OnPickDataAssetClicked(TSharedRef<IPropertyHandle> PropertyHandle, FInventoryItemEntry* CurrentEntry, UObject* CurrentOuter)
{
	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.Filter.ClassPaths.Add(UInventoryItemDefinition::StaticClass()->GetClassPathName());
	AssetPickerConfig.Filter.bRecursiveClasses = true;
	AssetPickerConfig.SelectionMode = ESelectionMode::Single;
	AssetPickerConfig.bAllowNullSelection = false;

	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateLambda([PropertyHandle, CurrentEntry, CurrentOuter](const FAssetData& AssetData)
	{
		UObject* SelectedObj = AssetData.GetAsset();
		if (UInventoryItemDefinition* SelectedDataAsset = Cast<UInventoryItemDefinition>(SelectedObj))
		{
			if (CurrentEntry)
			{
				CurrentEntry->EmptySlot();
				CurrentEntry->ItemDefinition = SelectedDataAsset;
				CurrentEntry->ItemStack = 1;
				CurrentEntry->ItemInstance = UInventoryItemInstance::NewItemInstance(CurrentOuter, SelectedDataAsset);
				PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
			}
		}
 	
		FSlateApplication::Get().DismissAllMenus();
	});
 
	TSharedRef<SWindow> PickerWindow = SNew(SWindow)
		.Title(LOCTEXT("PickDataAssetWindow", "选择 DataAsset"))
		.ClientSize(FVector2D(800, 600))
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		[
			SNew(SBox)
			.WidthOverride(800)
			.HeightOverride(600)
			[
				FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser")
				.Get().CreateAssetPicker(AssetPickerConfig)
			]
		];
 
	FSlateApplication::Get().AddWindow(PickerWindow);
 
	return FReply::Handled();
}

void FItemEntryPropertyTypeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle,
                                                          FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(400)
	[
		SNew(SButton)
		.Text(FText::FromString(TEXT("Init")))
		.OnClicked_Lambda([this, PropertyHandle]()
		{
			// 1. 获取 StructProperty
			FStructProperty* StructProp =
				CastField<FStructProperty>(PropertyHandle->GetProperty());

			if (!StructProp || StructProp->Struct != FInventoryItemEntry::StaticStruct())
			{
				return FReply::Handled();
			}

			// 2. 获取结构体内存
			void* RawData = nullptr;
			PropertyHandle->GetValueData(RawData);

			if (!RawData)
			{
				return FReply::Handled();
			}

			// 3. 转换为你的结构体并调用函数
			TArray<UObject*> OuterObjects;
			PropertyHandle->GetOuterObjects(OuterObjects);
			if (OuterObjects.Num() > 0 && OuterObjects[0])
			{
				OnPickDataAssetClicked(PropertyHandle, static_cast<FInventoryItemEntry*>(RawData), OuterObjects[0]);
			}
			
			return FReply::Handled();
		})
	];
}

void FItemEntryPropertyTypeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	uint32 NumChildren = 0;
	PropertyHandle->GetNumChildren(NumChildren);
 
	for (uint32 ChildIdx = 0; ChildIdx < NumChildren; ChildIdx++)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(ChildIdx);
		if (ChildHandle.IsValid())
		{
			ChildBuilder.AddProperty(ChildHandle.ToSharedRef());
		}
	}
}

#undef LOCTEXT_NAMESPACE