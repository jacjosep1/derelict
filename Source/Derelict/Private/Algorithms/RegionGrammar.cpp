// Fill out your copyright notice in the Description page of Project Settings.


#include "Algorithms/RegionGrammar.h"
#include "Algorithms/array2D.h"
#include "Util/DebugPrinting.h"

#include <queue>

void RegionGrammar::Generate_Graph() {
	graph_template_t START_1{
		"  e  ",
		"  v  ",
		"Hhhht",
		"  |  ",
		"  o  ",
	};

	// Generate default initial graph
	if (DEBUG_MESSAGES) GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, TEXT("Generating initial graph"));
	ConvertToGraphParams starting_params;
	starting_params.depth = 0;
	starting_params.rotated = false;
	starting_params.connectorL = nullptr;
	starting_params.connectorR = nullptr;
	graph = ConvertToGraph(START_1, starting_params); 
	if (DEBUG_MESSAGES) DebugPrint();

	// Loop until there are no fillers. (i.e. depth has reached max)
	for (int depth = 1; depth <= settings.max_depth; depth++) {
		graph_t graph_old = graph;
		// Find fillers in graph
		for (size_t i = 0; i < graph_old.size(); i++) {
			auto &node = graph_old[i];
			if (!Preset_Grammar_Gen::is_filler(node->region_label)) continue; // Skip over non-fillers

			// Choose a random graph to fill with at uniform random (TODO: weighted random). 
			// MUST NOT contain fillers if at max_depth. 
			// MUST contain fillers if NOT at max_depth. 
			const graph_ruleset_t& rules = (depth == settings.max_depth)
				? Preset_Grammar_Gen::rules_no_fillers : Preset_Grammar_Gen::rules_fillers;

			const RegionLabel& filler = node->region_label;
			const std::vector<graph_template_t>& sample_space = Preset_Grammar_Gen::access_ruleset(rules, filler);
			const graph_template_t& selected_templ = sample_space[FMath::RandRange(0, sample_space.size() - 1)];

			// Form connections, initialize graph params
			bool vertical = Preset_Grammar_Gen::is_vertical_filler(filler);
			ConvertToGraphParams graph_params;
			graph_params.depth		= depth;
			graph_params.rotated	= vertical;
			graph_params.connectorL = vertical ? node->neighbors[E_TOP]    : node->neighbors[E_LEFT];
			graph_params.connectorR = vertical ? node->neighbors[E_BOTTOM] : node->neighbors[E_RIGHT];

			// Generate subgraph
			graph_t subgraph = ConvertToGraph(selected_templ, graph_params);

			// Remove filler from original graph
			graph.erase(std::remove(graph.begin(), graph.end(), node), graph.end());

			// Join subgraph
			for (auto &n : subgraph)
				graph.push_back(n);
		}
		if (DEBUG_MESSAGES) DebugPrint(true);
	}

	// Generate locations
	for (const auto& node : graph)
		node->visited = false;
	std::queue<std::shared_ptr<Node>> search_queue;
	search_queue.push(graph.at(0));
	graph.at(0)->visited = true;

	while (!search_queue.empty()) {
		// Get next searched element and remove from queue
		std::shared_ptr<Node> current = search_queue.front();
		search_queue.pop();

		// Loop over unvisited neighbors
		for (auto [dir, neighbor] : current->neighbors) {
			if (!neighbor || neighbor->visited) continue;
			neighbor->location = current->location + dir;
			neighbor->visited = true;
			search_queue.push(neighbor);
		}
	}

	// Get graph bounds
	GenerateBounds();

	if (DEBUG_MESSAGES) DebugPrint();
}

RegionGrammar::bounds_t RegionGrammar::GenerateBounds() {
	location_t lbound{ MAX_INT32, MAX_INT32 };
	location_t ubound{ MIN_int32, MIN_int32 };
	for (const auto& node : graph) {
		if (node->location.x > ubound.x) ubound.x = node->location.x;
		if (node->location.y > ubound.y) ubound.y = node->location.y;
		if (node->location.x < lbound.x) lbound.x = node->location.x;
		if (node->location.y < lbound.y) lbound.y = node->location.y;
	}
	generated_bounds = { lbound, ubound };
	return generated_bounds;
}

void RegionGrammar::DebugPrint(bool skip) const {
	if (!skip) {
		FString s = TEXT("Graph size: ");
		s.AppendInt(graph.size());
		GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, s);
		if (graph.size() == 0) return;
		std::unordered_map<EDir, std::string, EDirHash> dir_names{
				{E_TOP, "T:"},
				{E_BOTTOM, "B:"},
				{E_LEFT, "L:"},
				{E_RIGHT, "R:"},
		};

		for (const auto& node : graph) {
			FString b = TEXT("Node: ");
			b.Append(LexToString(reinterpret_cast<size_t>(node.get()) & 0xFFFF));
			b.Append(",");
			b.AppendChar(static_cast<char>(node->region_label));
			b.Append(",");
			b.AppendInt(node->location.x);
			b.Append(",");
			b.AppendInt(node->location.y);
			b.Append("; Neighbors: ");
			std::vector<EDir> dirs{ E_TOP, E_BOTTOM, E_LEFT, E_RIGHT };
			for (const auto& dir : dirs) {
				auto& target = node->neighbors[dir];
				b.Append(UTF8_TO_TCHAR(dir_names[dir].c_str()));
				if (target) {
					b.AppendChar(static_cast<char>(target->region_label));
					b.Append(",");
					b.AppendInt(target->location.x);
					b.Append(",");
					b.AppendInt(target->location.y);
				}
				else {
					b.Append(" , , ");
				}
				b.Append(" ");
			}
			b.Append(";");
			GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, b);
		}
	}

	// Get graph bounds
	location_t lbound{ MAX_INT32, MAX_INT32 };
	location_t ubound{ MIN_int32, MIN_int32 };
	for (const auto& node : graph) {
		if (node->location.x > ubound.x) ubound.x = node->location.x;
		if (node->location.y > ubound.y) ubound.y = node->location.y;
		if (node->location.x < lbound.x) lbound.x = node->location.x;
		if (node->location.y < lbound.y) lbound.y = node->location.y;
	}

	// Physical graph
	for (int r = lbound.x; r <= ubound.x; r++) {
		FString line = TEXT(" ");
		for (int c = lbound.y; c <= ubound.y; c++) {
			char label = ' ';
			for (const auto& node : graph)
				if (node->location == location_t{ r,c }) {
					label = static_cast<char>(node->region_label);
					break;
				}
			line.AppendChar(label);
		}
		GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::White, line);
	}
		
}

RegionGrammar::graph_t RegionGrammar::ConvertToGraph(const graph_template_t& templ, 
	const RegionGrammar::ConvertToGraphParams& params) {
	RegionGrammar::graph_t out;
	Array2D<std::shared_ptr<Node>> temp_space(
		params.rotated ? templ[0].length() : templ.size(),
		params.rotated ? templ.size() : templ[0].length(),
		nullptr
	);
	bool hit_first_connector = false;

	int row = 0;
	for (const std::string &line : templ) {
		int col = 0;
		for (const char& c : line) {
			
			auto label = static_cast<RegionLabel>(c);

			location_t location; // new vertical, horizontal location (local)
			if (params.rotated) location = { col, row };
			else location = { row, col };

			if (label == RegionLabel::none) {
				temp_space.get(location).reset();
				col++; continue; // Ignore blank spots
			}

			// Flip horizontal & vertical fillers
			if (params.rotated && Preset_Grammar_Gen::is_filler(label)) {
				label = Preset_Grammar_Gen::alternate_filler(label);
			}

			if (DEBUG_MESSAGES) DebugPrinting::PrintLocation(location, "CurrentLoc");

			std::shared_ptr<Node> added;
			if (label == RegionLabel::connector) {  // Link connectors back to the passed connectors

				if (DEBUG_MESSAGES) {
					if (hit_first_connector) DebugPrinting::PrintShared(params.connectorR, "connectorR:");
					if (!hit_first_connector) DebugPrinting::PrintShared(params.connectorL, "connectorL:");
				}

				added = hit_first_connector ? params.connectorR : params.connectorL;
				if (!hit_first_connector) { // This is connectorL
					hit_first_connector = true;
				}
			}
			else { // Add new node
				added = std::make_shared<Node>(label, params.depth);
				out.push_back(added);
			}

			// Link back references to top & left (previously generated)
			temp_space.get(location) = added;
			auto top_cpy  = temp_space.get_copy(location + E_TOP);
			auto left_cpy = temp_space.get_copy(location + E_LEFT);

			if (top_cpy) {
				auto top_ref = temp_space.get(location + E_TOP);
				if (added) added->neighbors[E_TOP] = top_ref;
				if (top_ref) top_ref->neighbors[E_BOTTOM] = added;
			}
			if (left_cpy) {
				auto left_ref = temp_space.get(location + E_LEFT);

				if (added) added->neighbors[E_LEFT] = left_ref;
				if (left_ref) left_ref->neighbors[E_RIGHT] = added;
			}

			if (DEBUG_MESSAGES) {
				FString ss = TEXT("");
				ss.Append(LexToString(reinterpret_cast<size_t>(added.get()) & 0xFFFF));
				ss.AppendChar(static_cast<char>(c));
				GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, ss);
				for (size_t i = 0; i < temp_space.height; i++) {
					FString f = TEXT("");
					for (size_t j = 0; j < temp_space.width; j++) {
						f.Append(LexToString(reinterpret_cast<size_t>(temp_space.get(i, j).get()) & 0xFFFF));
						f.Append(" ");
					}
					GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Blue, f);
				}
				GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, TEXT(""));
			}

			col++;
		}
		row++;
	}

	return out;
}