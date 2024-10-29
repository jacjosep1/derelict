// Fill out your copyright notice in the Description page of Project Settings.


#include "Algorithms/RegionGrammar.h"
#include "Algorithms/array2D.h"

void RegionGrammar::Generate_Graph() {
	
	// Generate default initial graph
	graph = ConvertToGraph(Preset_Grammar_Gen::START_1);

	//TODO
}

void RegionGrammar::DebugPrint() const {
	// Find max height and width
	location_t bounds{ 0, 0 };
	for (const Node& node : graph) {
		if (node.location.x > bounds.x) bounds.x = node.location.x;
		if (node.location.y > bounds.y) bounds.y = node.location.y;
	}

	// Print
	for (int row = 0; row < bounds.x; row++) {
		FString f = TEXT("");
		for (int col = 0; col < bounds.y; col++) {
			// Find location
			char p = ' ';
			for (const Node& node : graph)
				if (node.location == location_t{ row, col })
					p = static_cast<char>(node.region_label);
			f.AppendChar(p);
		}
		GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, f);
	}
}

std::vector<RegionGrammar::Node> RegionGrammar::ConvertToGraph(const graph_template_t& templ, bool rotated, 
	Node* connectorL, Node* connectorR) {

	std::vector<Node> out;
	Array2D<Node*> temp_space(templ[0].length(), templ.size(), nullptr);
	bool hit_first_connector = false;
	location_t connectorL_local_location{ 0, 0 };

	int row = 0;
	for (const std::string &line : templ) {
		int col = 0;
		for (const char& c : line) {

			auto label = static_cast<RegionLabel>(c);
			if (label == RegionLabel::none) continue; // Ignore blank spots

			location_t location; // new vertical, horizontal location (local)
			if (rotated) location = { col, row };
			else         location = { row, col };

			// Link connectors back to the passed connectors
			if (label == RegionLabel::connector) { 
				temp_space.get(location) = hit_first_connector ? connectorR : connectorL;
				if (!hit_first_connector) { // This is connectorL
					connectorL_local_location = location;
					hit_first_connector = true;
				}
				continue; // Dont add the connector as a new node, it references the passed nodes. 
			}

			// Add new node
			out.emplace_back(label, location);
			auto &added = out.back();

			// Link back references to top & left (previously generated)
			temp_space.get(location) = &added;
			auto& top_ref = temp_space.get(location + E_TOP);
			auto& left_ref = temp_space.get(location + E_LEFT);
			added.neighbors[E_TOP]  = top_ref;
			added.neighbors[E_LEFT] = left_ref;
			if (top_ref)  top_ref->neighbors[E_BOTTOM] = &added;
			if (left_ref) left_ref->neighbors[E_RIGHT] = &added;

			col++;
		}
		row++;
	}

	if (connectorL) { // Use left connector for global location offset
		// global location offset (temp_space[0,0] location) = cL's global - cL's local
		const location_t& global_location_offset = connectorL->location - connectorL_local_location;

		// Apply transformation
		for (Node &e : out) e.location = e.location + global_location_offset;
	}
	return out;
}