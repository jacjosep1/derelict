// Fill out your copyright notice in the Description page of Project Settings.

#include "Algorithms/AlgorithmTester.h"
#include "Algorithms/WFC_Interface.h"

TArray<int32> UAlgorithmTester::SimpleImageWFC(int32 SizeX, int32 SizeY, TArray<int32> Seed, int32 SeedDimension) {
    TArray2D<int32> n_seed({ SeedDimension, SeedDimension });
    n_seed.data = Seed;

    auto tmp = WFC_Interface::Generate_WFC_Region(n_seed, location_t{ SizeX, SizeY });
    return tmp.data;
}