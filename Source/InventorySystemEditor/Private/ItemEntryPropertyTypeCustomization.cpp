// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemEntryPropertyTypeCustomization.h"

#include "ContentBrowserModule.h"
#include "DetailWidgetRow.h"
#include "IContentBrowserSingleton.h"
#include "IDetailChildrenBuilder.h"
#include "InventoryItemDefinition.h"
#include "InventorySlotData.h"
#include "Processors/InventoryProcessor_Stackable.h"

#define LOCTEXT_NAMESPACE "FItemEntryPropertyTypeCustomization"

TSharedRef<IPropertyTypeCustomization> FItemEntryPropertyTypeCustomization::MakeInstance()
{
	return MakeShared<FItemEntryPropertyTypeCustomization>();
}

FReply FItemEntryPropertyTypeCustomization::OnPickDataAssetClicked(TSharedRef<IPropertyHandle> PropertyHandle)
{
	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.Filter.ClassPaths.Add(UInventoryItemDefinition::StaticClass()->GetClassPathName());
	AssetPickerConfig.Filter.bRecursiveClasses = true;
	AssetPickerConfig.SelectionMode = ESelectionMode::Single;
	AssetPickerConfig.bAllowNullSelection = false;

	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateLambda([PropertyHandle](const FAssetData& AssetData)
	{
		UObject* SelectedObj = AssetData.GetAsset();
		if (UInventoryItemDefinition* SelectedDataAsset = Cast<UInventoryItemDefinition>(SelectedObj))
		{
			// 修复：非模态窗口打开期间属性或数组可能重建，选择时重新解析地址。
			void* RawData = nullptr;
			TArray<UObject*> Owners;
			PropertyHandle->GetOuterObjects(Owners);
			if (PropertyHandle->IsValidHandle() && Owners.Num() == 1 && IsValid(Owners[0])
				&& PropertyHandle->GetValueData(RawData) == FPropertyAccess::Success && RawData)
			{
				auto* CurrentEntry = static_cast<FInventoryItemEntry*>(RawData);
				CurrentEntry->EmptySlot();
				CurrentEntry->ItemDefinition = SelectedDataAsset;
				if (SelectedDataAsset)
				{
					if (const auto CDO = CurrentEntry->ItemDefinition->GetSlotDataDefinitionCDO())
					{
						CurrentEntry->ItemSlotData = CDO->GetDefaultSlotData(SelectedDataAsset, Owners[0]);
					}
				}
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
		.OnClicked_Lambda([PropertyHandle]()
		{
			return OnPickDataAssetClicked(PropertyHandle);
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