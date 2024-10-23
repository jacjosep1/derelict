// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CommonStructs.generated.h"

USTRUCT(BlueprintType)
struct FImageCSV_Row : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString V;
};

/**
 * 
 */
UCLASS()
class DERELICT_API UCommonStructs : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
};
