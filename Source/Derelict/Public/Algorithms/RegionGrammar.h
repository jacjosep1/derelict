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
		Node(const RegionLabel& _r, location_t  _l, int _d=0)
			: region_label{ _r }, location{ _l }, depth{ _d } {}

		RegionLabel region_label{ RegionLabel::none };
		location_t location{ 0, 0 };
		int depth{ 0 };

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

	void DebugPrint() const;

private:
	// Converts a graph template to an actual graph section
	// By default graphs will be left-to-right. Set rotated=true to flip the template to up-to-down. 
	static std::vector<Node> ConvertToGraph(const graph_template_t& templ, bool rotated=false,
		Node* connectorL = nullptr, Node* connectorR = nullptr);

};
