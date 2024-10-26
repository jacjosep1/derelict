// Fill out your copyright notice in the Description page of Project Settings.

#include "Algorithms/AlgorithmTester.h"
#include "Algorithms/WFC_Interface.h"

void UAlgorithmTester::SimpleImageWFC(int32 SizeX, int32 SizeY, UDataTable* SeedData, FString CharSet) {
    WFC_Interface::SetupCharSet(CharSet);

    auto seed = WFC_Interface::ReadImage_CSV(SeedData);
    auto generated = WFC_Interface::Generate_WFC_Region(seed, location_t{ SizeX, SizeY });
    generated.DebugPrint();

}