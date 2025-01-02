// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Algorithms/PerlinNoiseBPLibrary.h"
#include "Algorithms/array2d.h"

#include "AlgorithmTester.generated.h"

namespace FastNoiseContainer {
	class FastNoiseInstance {
		int seed;
		float scale;
	public:
		FastNoiseInstance(float scale_) : scale(scale_) {
			seed = FMath::RandRange(0, 10000);
		}

		float GetValue(int x, int y) {
			UPerlinNoiseBPLibrary::SetSeed(seed);
			return UPerlinNoiseBPLibrary::TwoD_Perlin_Noise(x, y, scale, 1);
		}
	};

	static FastNoiseInstance derelictness{.2};
};

UENUM(BlueprintType)
enum class BP_Dir : uint8
{
	None UMETA(DisplayName = "None"),
	Top UMETA(DisplayName = "Top"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),
	Bottom UMETA(DisplayName = "Bottom")
};

USTRUCT(BlueprintType)
struct FGenOutput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
    FString Label;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
    int32 Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
    FString LeftLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
	FString RightLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
	FString UpLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
	FString DownLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
	bool HasTurret;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
	FString Region_Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
	TArray<float> UniformProcess;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
	BP_Dir WindowPlacement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
	float derelictness;

	FGenOutput() : Label( TEXT("E") ), LeftLabel(TEXT("E")), RightLabel( TEXT("E") ), UpLabel( TEXT("E") ), DownLabel( TEXT("E") ), 
		HasTurret(false), Region_Label(" "), UniformProcess{}, WindowPlacement(BP_Dir::None) {
	}

};

USTRUCT(BlueprintType)
struct FWFCOutput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
	int32 ExtentX_min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
	int32 ExtentY_min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
	int32 ExtentX_max;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gen Testing")
	int32 ExtentY_max;

};

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FMyEventDelegate, int32, X, int32, Y, FGenOutput, GenOutput);

UCLASS()
class DERELICT_API UAlgorithmTester : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Gen Testing")
	static void SimpleImageWFC(int32 SizeX, int32 SizeY, UDataTable* SeedData);

	UFUNCTION(BlueprintCallable, Category = "Gen Testing")
	static void SimpleGrammar();

	UFUNCTION(BlueprintCallable, Category = "Gen Testing")
	static FWFCOutput TestGrammarToWFC(FMyEventDelegate delegate, int32 RegionSize, int32 GrammarDepth);

private:
	static TArray<float> GenerateRandomFloats(int count);
	static BP_Dir ConvertDir(const EDir& dir);
};
