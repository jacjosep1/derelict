// Fill out your copyright notice in the Description page of Project Settings.

#include "Algorithms/AlgorithmTester.h"
#include "Algorithms/Presets/Preset_WFC_Gen.h"
#include "Algorithms/WFC_Interface.h"

void UAlgorithmTester::SimpleImageWFC(int32 SizeX, int32 SizeY, UDataTable* SeedData) {
    WFC_Interface<PRESET_MediumHalls> wfc;

    auto seed = wfc.ReadImage_CSV(SeedData);
    auto generated = wfc.Generate_WFC_Region(seed, location_t{ SizeX, SizeY });
    generated.DebugPrint();

}