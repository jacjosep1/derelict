// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Algorithms/WFC_Interface.h"
#include "Algorithms/Presets/Preset_Grammar_Gen.h"

#include <unordered_map>
#include <vector>

// Input properties
struct RegionGrammarSettings {
	int max_depth{ 3 }; // Max number of replacements from the default graph initialization
};

// Class to handle initial game objective and ship layout
class RegionGrammar
{
	// Node structure to represent future regions
	struct Node {
		RegionLabel region_label{ RegionLabel::ship_medium_halls };
		std::unordered_map<EDir, Node*> neighbors {
			{E_TOP, nullptr},
			{E_BOTTOM, nullptr},
			{E_LEFT, nullptr},
			{E_RIGHT, nullptr},
		};
	};

	// Spatial representation of general ship and mission layout
	std::vector<Node> graph;

	// Input properties
	const RegionGrammarSettings settings;

public:
	RegionGrammar() : settings{} {};
	RegionGrammar(const RegionGrammarSettings& _settings) : settings{ _settings } {};

	// Generate the mission objective spatial graph
	void Generate_Graph();

};
