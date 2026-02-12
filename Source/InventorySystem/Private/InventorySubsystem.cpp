// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySubsystem.h"

#include "InventoryContainerComponent.h"
#include "InventoryUserWidget.h"

void UInventorySubsystem::Deinitialize()
{
	Super::Deinitialize();

	for (auto Itr : WidgetPool)
	{
		Itr.Value.ReleaseAllSlateResources();
	}

	WidgetPool.Empty();
}

FUserWidgetPool* UInventorySubsystem::GetOrAddWidgetPool(const TSubclassOf<UInventoryUserWidget>& WidgetClass)
{
	if (const auto Found = WidgetPool.Find(WidgetClass))
	{
		return Found;
	}

	auto* New = &WidgetPool.Add(WidgetClass);
	New->SetWorld(GetWorld());
	New->SetDefaultPlayerController(GetLocalPlayer()->PlayerController);
	return New;
}

UInventoryUserWidget* UInventorySubsystem::RequestInventoryUserWidget(TSubclassOf<UInventoryUserWidget> InWidgetClass,
	UInventoryContainerComponent* InContainer, int Index)
{
	if (!InWidgetClass || !InContainer)
	{
		return nullptr;
	}

	const auto Pool = GetOrAddWidgetPool(InWidgetClass);
	const auto WidgetInstance = Pool->GetOrCreateInstance(InWidgetClass);
	WidgetInstance->InitParams(InContainer, Index);
	WidgetInstance->NativePostInventoryInitialize();

	return WidgetInstance;
}

void UInventorySubsystem::ReleaseInventoryUserWidget(UInventoryUserWidget* InInventoryUserWidget)
{
	if (InInventoryUserWidget)
	{
		const auto WidgetClass = InInventoryUserWidget->GetClass();
		if (const auto Pool = GetOrAddWidgetPool(WidgetClass))
		{
			InInventoryUserWidget->NativePreReleaseWidgetPool();
			InInventoryUserWidget->RemoveFromParent();
			Pool->Release(InInventoryUserWidget);
		}
	}
}

void UInventorySubsystem::ReleasePool(TSubclassOf<UInventoryUserWidget> Class)
{
	if (const auto Found = WidgetPool.Find(Class))
	{
		Found->ReleaseAllSlateResources();
		WidgetPool.Remove(Class);
	}
}
