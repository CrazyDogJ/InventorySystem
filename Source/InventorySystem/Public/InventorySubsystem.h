// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidgetPool.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InventorySubsystem.generated.h"

class UInventoryItemInstance;
class UInventoryContainerComponent;
class UInventoryUserWidget;

UCLASS()
class INVENTORYSYSTEM_API UInventorySubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	TMap<TSubclassOf<UInventoryUserWidget>, FUserWidgetPool> WidgetPool;

	virtual void Deinitialize() override;
	
private:
	FUserWidgetPool* GetOrAddWidgetPool(const TSubclassOf<UInventoryUserWidget>& WidgetClass);
	
public:
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "InWidgetClass"))
	UInventoryUserWidget* RequestInventoryUserWidget(TSubclassOf<UInventoryUserWidget> InWidgetClass,
		UInventoryContainerComponent* InContainer, int Index = -1);
	
	UFUNCTION(BlueprintCallable)
	void ReleaseInventoryUserWidget(UInventoryUserWidget* InInventoryUserWidget);

	UFUNCTION(BlueprintCallable)
	void ReleasePool(TSubclassOf<UInventoryUserWidget> Class);
};
