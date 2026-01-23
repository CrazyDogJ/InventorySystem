#include "InventorySystemEditor.h"

#include "InventorySystemSettings.h"
#include "ISettingsModule.h"
#include "ItemEntryPropertyTypeCustomization.h"

#define LOCTEXT_NAMESPACE "FInventorySystemEditorModule"

void FInventorySystemEditorModule::StartupModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Inventory System",
			LOCTEXT("RuntimeSettingsName", "Inventory System"),
			LOCTEXT("RuntimeSettingsDescription", "Configure Inventory System settings"),
			GetMutableDefault<UInventorySystemSettings>());
	}
	
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FInventoryItemEntry::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FItemEntryPropertyTypeCustomization::MakeInstance)
	);
}

void FInventorySystemEditorModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Inventory System");
	}
	
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout(FInventoryItemEntry::StaticStruct()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FInventorySystemEditorModule, InventorySystemEditor)