// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystemSettings.h"
#include "InventoryUserWidget.h"

TMap<TSoftClassPtr<UInterface>, TSoftClassPtr<UInventoryUserWidget>> UInventorySystemSettings::GetAllUserWidgets()
{
	TMap<TSoftClassPtr<UInterface>, TSoftClassPtr<UInventoryUserWidget>> Widgets;
	Widgets.Append(UserWidgets);
	for (auto Itr : UserWidgets_Cpp)
	{
		Widgets.Add(TSoftClassPtr<UInterface>(Itr.Key), Itr.Value);
	}

	return Widgets;
}

TSubclassOf<UInventoryUserWidget> UInventorySystemSettings::GetUserWidgetClass(TSubclassOf<UInterface> InInterface)
{
	auto Mappings = GetAllUserWidgets();
	if (const auto Found = Mappings.Find(TSoftClassPtr<UInterface>(InInterface)))
	{
		return Found->LoadSynchronous();
	}

	return nullptr;
}
