// Fill out your copyright notice in the Description page of Project Settings.

#include "Algorithms/AlgorithmTester.h"
#include "Algorithms/WFC_Interface.h"

TArray<FString> UAlgorithmTester::SimpleImageWFC(int32 SizeX, int32 SizeY, TArray<FString> Seed) {

    TArray<TCHAR> data;
    int32 InputX = 0;
    int32 InputY = Seed.Num();
    TArray2D<TCHAR> n_seed({ InputX, InputY }, '0');

    for (int32 y = 0; y < InputY; y++) {
        const auto& SeedLine = Seed[y];
        if (InputX != 0) {
            check(SeedLine.Len() == InputX);
        }
        else {
            InputX = SeedLine.Len();
        }

        for (int32 x = 0; x < InputX; x++) {
            n_seed.Get({x, y}) = SeedLine[x];
        }
    }

    auto generated = WFC_Interface::Generate_WFC_Region(n_seed, location_t{ SizeX, SizeY });

    TArray<FString> out;
    out.Init("", SizeY);
    for (int32 y = 0; y < SizeY; y++) {
        for (int32 x = 0; x < SizeX; x++)
            out[y].Append(FString(1, &generated.Get({ x, y })));
    }
    return out;
}