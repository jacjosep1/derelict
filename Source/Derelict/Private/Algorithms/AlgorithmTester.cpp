// Fill out your copyright notice in the Description page of Project Settings.

#include "Algorithms/AlgorithmTester.h"
#include "Algorithms/Presets/Preset_WFC_Gen.h"
#include "Algorithms/WFC_Interface.h"
#include "Algorithms/RegionGrammar.h"
#include "Util/DebugPrinting.h"
#include "Math/UnrealMathUtility.h"

void UAlgorithmTester::SimpleImageWFC(int32 SizeX, int32 SizeY, UDataTable* SeedData) {
    WFC_Interface<PRESET_MediumHalls> wfc;

    auto seed = wfc.ReadImage_CSV(SeedData);
    auto [generated, properties] = wfc.Generate_WFC_Region(seed, location_t{ SizeX, SizeY }, {E_TOP, E_LEFT});
    generated.DebugPrint();
}

void UAlgorithmTester::SimpleGrammar() {
    RegionGrammar grammar;
    grammar.Generate_Graph();
    grammar.DebugPrint();
}

TArray<float> UAlgorithmTester::GenerateRandomFloats(int count) {
    TArray<float> randomFloats;
    randomFloats.Reserve(count);
    for (int i = 0; i < count; i++) randomFloats.Add(FMath::FRand());
    return randomFloats;
}

BP_Dir UAlgorithmTester::ConvertDir(const EDir &dir) {
    if (dir == E_TOP) return BP_Dir::Top;
    if (dir == E_BOTTOM) return BP_Dir::Bottom;
    if (dir == E_LEFT) return BP_Dir::Left;
    if (dir == E_RIGHT) return BP_Dir::Right;
    return BP_Dir::None;
}

FWFCOutput UAlgorithmTester::TestGrammarToWFC(FMyEventDelegate delegate, int32 RegionSize, int32 GrammarDepth) {
    FWFCOutput output{0,0,0,0};
    location_t min_bounds = MAX_LOCATION_T;
    location_t max_bounds = MIN_LOCATION_T;

    // Load Seeds
    for (const auto& [region, pair] : WFC_SPECIFICATIONS) {
        std::visit([&](auto&& s) {
            auto& non_const_s = const_cast<std::remove_const_t<std::remove_reference_t<decltype(s)>>&>(s);
            auto& path_prop = SEED_PATHS[pair.properties.seed_table_id];

            path_prop.load();
            non_const_s.seed = non_const_s.generator.ReadImage_CSV(path_prop.table);
            check(non_const_s.seed.width > 0);
            check(non_const_s.seed.height > 0);
        }, pair.spec);
    }

    // Generate mission graph
    RegionGrammarSettings grammar_settings;
    grammar_settings.max_depth = GrammarDepth;
    RegionGrammar grammar(grammar_settings);
    grammar.Generate_Graph();
    grammar.DebugPrint();
    RegionGrammar::graph_t graph = grammar.GetGraph();
    DebugPrinting::PrintInt(graph.size(), "GRAPH SIZE: ");

    // Perform WFC on each graph node
    const location_t GRID_SIZE = { RegionSize, RegionSize };
    for (const auto& node : graph) {
        if (!node->visited) continue;

        std::vector<EDir> exit;
        exit.reserve(4);

        for (const auto& [dir, neightbor] : node->neighbors) {
            if (neightbor) exit.push_back(dir);
        }

        // Fill with WFC specified by region label. 
        std::visit([&](auto&& s) {
            location_t modified_grid = GRID_SIZE / s.scale;
            auto [generated, property_matrix] = s.generator.Generate_WFC_Region(s.seed, modified_grid, exit, node->region_label);

            location_t offset = node->location * GRID_SIZE;

            for (int32 i = 0; i < generated.height; i++) for (int32 j = 0; j < generated.width; j++) {
                const TCHAR& item = generated.get(i, j);
                const auto& property = property_matrix.get(i, j);
                if (delegate.IsBound()) {
                    location_t global_location = (location_t{ i, j } * s.scale) + offset;
                    FString LabelStr(1, &item);

                    FGenOutput output;
                    output.Label = LabelStr;
                    output.Scale = s.scale;
                    output.HasTurret = property.turret_level;
                    output.UniformProcess = GenerateRandomFloats(20);
                    output.derelictness = FastNoiseContainer::derelictness.GetValue(i, j);

                    // Edge detection for windows
                    for (const auto& [dir, is_edge] : property.edge_indicators)
                        if (is_edge) {
                            bool found = false;
                            for (const auto& local_dir : node->GetOpenSides()) // make sure edge matches with an open side of the region
                                if (local_dir == dir) {
                                    found = true;
                                    break;
                                }
                            if (found) {
                                output.WindowPlacement = ConvertDir(dir);
                                break;
                            }
                        }

                    char converted_char = static_cast<char>(node->region_label);
                    FString ConvertedString(1, &converted_char);
                    output.Region_Label = ConvertedString;

                    // Get neighbors
                    auto LeftOpt = generated.get_copy({ i - 1, j });
                    auto RightOpt = generated.get_copy({ i + 1, j });
                    auto UpOpt = generated.get_copy({ i, j - 1 });
                    auto DownOpt = generated.get_copy({ i, j + 1 });
                    if (LeftOpt)    output.LeftLabel    = FString(1, &*LeftOpt);
                    if (RightOpt)   output.RightLabel   = FString(1, &*RightOpt);
                    if (UpOpt)      output.UpLabel      = FString(1, &*UpOpt);
                    if (DownOpt)    output.DownLabel    = FString(1, &*DownOpt);

                    // Update Bounds
                    if (global_location.x < min_bounds.x) min_bounds.x = global_location.x;
                    if (global_location.y < min_bounds.y) min_bounds.y = global_location.y;
                    if (global_location.x > max_bounds.x) max_bounds.x = global_location.x;
                    if (global_location.y > max_bounds.y) max_bounds.y = global_location.y;

                    delegate.Execute(global_location.x, global_location.y, output);
                }
            }
        }, WFC_SPECIFICATIONS[node->region_label].spec);

    }

    output.ExtentX_min = min_bounds.x;
    output.ExtentX_max = max_bounds.x;
    output.ExtentY_min = min_bounds.y;
    output.ExtentY_max = max_bounds.y;
    return output;
    
}