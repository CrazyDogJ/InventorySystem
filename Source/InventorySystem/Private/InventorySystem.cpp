// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventorySystem.h"

#define LOCTEXT_NAMESPACE "FInventorySystemModule"

DEFINE_LOG_CATEGORY(LogInventorySystem);

void FInventorySystemModule::StartupModule()
{
}

void FInventorySystemModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInventorySystemModule, InventorySystem)