// Fill out your copyright notice in the Description page of Project Settings.

#include "Algorithms/AlgorithmTester.h"
#include "Algorithms/Presets/Preset_WFC_Gen.h"
#include "Algorithms/WFC_Interface.h"
#include "Algorithms/RegionGrammar.h"
#include "Util/DebugPrinting.h"

void UAlgorithmTester::SimpleImageWFC(int32 SizeX, int32 SizeY, UDataTable* SeedData) {
    WFC_Interface<PRESET_MediumHalls> wfc;

    auto seed = wfc.ReadImage_CSV(SeedData);
    auto generated = wfc.Generate_WFC_Region(seed, location_t{ SizeX, SizeY }, {E_TOP, E_LEFT});
    generated.DebugPrint();
}

void UAlgorithmTester::SimpleGrammar() {
    RegionGrammar grammar;
    grammar.Generate_Graph();
    grammar.DebugPrint();
}

void UAlgorithmTester::TestGrammarToWFC(FMyEventDelegate delegate) {
    UDataTable* DT_TestSeed;
    FSoftObjectPath UnitDataTablePath = FSoftObjectPath(TEXT("/Game/Data/Seeds/DT_TestSeed.DT_TestSeed"));
    DT_TestSeed = Cast<UDataTable>(UnitDataTablePath.ResolveObject());

    if (!DT_TestSeed) DT_TestSeed = Cast<UDataTable>(UnitDataTablePath.TryLoad());
    if (!DT_TestSeed) {
        DebugPrinting::PrintBool(false, "DT Failure. ");
        return;
    }

    WFC_Interface<PRESET_MediumHalls> wfc;
    auto TestSeed = wfc.ReadImage_CSV(DT_TestSeed);

    for (const auto& [region, spec] : WFC_SPECIFICATIONS) {
        std::visit([&](auto&& s) {
            auto& non_const_s = const_cast<std::remove_const_t<std::remove_reference_t<decltype(s)>>&>(s);
            non_const_s.seed = non_const_s.generator.ReadImage_CSV(DT_TestSeed);
        }, spec);
    }

    RegionGrammar grammar;
    grammar.Generate_Graph();
    grammar.DebugPrint();
    RegionGrammar::graph_t graph = grammar.GetGraph();
    DebugPrinting::PrintInt(graph.size(), "GRAPH SIZE: ");

    constexpr location_t GRID_SIZE = { 32, 32 };
    for (const auto& node : graph) {
        if (!node->visited) continue;
        //DebugPrinting::PrintLocation(node->location, "Debug: ");
        //DebugPrinting::PrintChar(static_cast<char>(node->region_label), "Debug: ");

        std::vector<EDir> exit;
        exit.reserve(4);

        for (const auto& [dir, neightbor] : node->neighbors) {
            if (neightbor) exit.push_back(dir);
        }

        // Fill with WFC specified by region label. 
        std::visit([&](auto&& s) {
            location_t modified_grid = GRID_SIZE / s.scale;
            auto generated = s.generator.Generate_WFC_Region(s.seed, modified_grid, exit);

            location_t offset = node->location * GRID_SIZE;

            for (int32 i = 0; i < generated.height; i++) for (int32 j = 0; j < generated.width; j++) {
                const TCHAR& item = generated.get(i, j);
                if (delegate.IsBound()) {
                    location_t global_location = (location_t{ i, j } * s.scale) + offset;
                    FString LabelStr(1, &item);
                    delegate.Execute(global_location.x, global_location.y, LabelStr, s.scale);
                }
            }
        }, WFC_SPECIFICATIONS[node->region_label]);

    }
    
}