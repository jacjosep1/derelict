// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Algorithms/WFC_Interface.h"
#include "Algorithms/Presets/Preset_Grammar_Gen.h"

#include <unordered_map>
#include <vector>
#include <memory>

// Input properties
struct RegionGrammarSettings {
	int max_depth{ 2 }; // Max number of replacements from the default graph initialization
};

// Class to handle initial game objective and ship layout
class RegionGrammar
{
	// Node structure to represent future regions
	struct Node {
		Node(const RegionLabel& _r, int _d=0)
			: region_label{ _r }, depth{ _d } {}

		RegionLabel region_label{ RegionLabel::none };
		int depth{ 0 };

		std::unordered_map<EDir, std::shared_ptr<Node>, EDirHash> neighbors {
			{E_TOP, nullptr},
			{E_BOTTOM, nullptr},
			{E_LEFT, nullptr},
			{E_RIGHT, nullptr},
		};

		// Traversal params
		bool visited{ false };
		location_t location{ 0, 0 };
	};

	// Spatial representation of general ship and mission layout
	typedef std::vector<std::shared_ptr<Node>> graph_t;
	graph_t graph;

	// Input properties
	const RegionGrammarSettings settings;

	// Debugging
	static constexpr bool DEBUG_MESSAGES{ true };

	static constexpr int32 MAX_INT32 = 0x7FFFFFFF;

public:
	RegionGrammar() : settings{} {};
	RegionGrammar(const RegionGrammarSettings& _settings) : settings{ _settings } {};

	// Generate the mission objective spatial graph
	void Generate_Graph();

	void DebugPrint() const;

private:
	// Converts a graph template to an actual graph section
	// By default graphs will be left-to-right. Set rotated=true to flip the template to up-to-down. 
	struct ConvertToGraphParams {
		bool rotated = false;
		std::shared_ptr<Node> connectorL = nullptr;
		std::shared_ptr<Node> connectorR = nullptr;
		int depth = 0;
	};
	static graph_t ConvertToGraph(const graph_template_t& templ, const ConvertToGraphParams &params);

};