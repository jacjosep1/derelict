// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AlgorithmTester.generated.h"

UCLASS()
class DERELICT_API UAlgorithmTester : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "WFC Testing")
	static TArray<FString> SimpleImageWFC(int32 SizeX, int32 SizeY, TArray<FString> Seed);
	
};
