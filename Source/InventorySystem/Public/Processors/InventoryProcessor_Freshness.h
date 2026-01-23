// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryProcessor_Freshness.generated.h"

USTRUCT(BlueprintType)
struct FFreshnessFragment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float Freshness = 1.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float LastUpdatePlayedTime = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float RotRate = 1.0f;
};
