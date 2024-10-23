// Fill out your copyright notice in the Description page of Project Settings.

#include "Algorithms/AlgorithmTester.h"
#include "Algorithms/WFC_Interface.h"

TArray<FString> UAlgorithmTester::SimpleImageWFC(int32 SizeX, int32 SizeY, UDataTable* SeedData, FString CharSet) {
    WFC_Interface::SetupCharSet(CharSet);

    auto seed = WFC_Interface::ReadImage_CSV(SeedData);
    auto generated = WFC_Interface::Generate_WFC_Region(seed, location_t{ SizeX, SizeY });
    generated.DebugPrint();

    TArray<FString> out;

    //out.Init("", SizeY);
    //for (int32 y = 0; y < SizeY; y++) for (int32 x = 0; x < SizeX; x++) {
    //    TCHAR character = generated.get({ x, y });
    //    out[y].Append(FString(1, &character));
    //}
    return out;
}