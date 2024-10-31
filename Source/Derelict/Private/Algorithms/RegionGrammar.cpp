// Fill out your copyright notice in the Description page of Project Settings.


#include "Algorithms/RegionGrammar.h"
#include "Algorithms/array2D.h"

void RegionGrammar::Generate_Graph() {
	
	// Generate default initial graph
	if (DEBUG_MESSAGES) GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, TEXT("Generating initial graph"));
	graph = ConvertToGraph(Preset_Grammar_Gen::START_1, ConvertToGraphParams());

	// Loop until there are no fillers. (i.e. depth has reached max)
	for (int depth = 0; depth <= settings.max_depth; depth++) {
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

			// Find subgraph size
			location_t subgraph_bounds = GraphSize(subgraph);

			// Push over all graph nodes to bottom/right (depending on vertical)
			if (vertical) {
				for (const auto& n : graph)
					if (n->location.y > node->location.y) n->location.y += subgraph_bounds.y;
			}
			else {
				for (const auto& n : graph)
					if (n->location.x > node->location.x) n->location.x += subgraph_bounds.x;
			}
			
			// Join subgraph
			for (auto &n : subgraph)
				graph.push_back(n);

			DebugPrint();
			if (DEBUG_MESSAGES) {
				FString bounds_str = TEXT("Subgraph size: ");
				bounds_str.AppendInt(subgraph_bounds.x);
				bounds_str.Append(", ");
				bounds_str.AppendInt(subgraph_bounds.y);
				GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, bounds_str);
			}
		}
	}

}

location_t RegionGrammar::GraphSize(const graph_t& g) {
	location_t Lbounds{ MAX_INT32, MAX_INT32 };
	location_t Ubounds{ 0, 0 };
	bool exists = false;
	for (const std::shared_ptr<Node>& node : g) {
		exists = true;
		if (node->location.x > Ubounds.x) Ubounds.x = node->location.x;
		if (node->location.y > Ubounds.y) Ubounds.y = node->location.y;
		if (node->location.x < Lbounds.x) Lbounds.x = node->location.x;
		if (node->location.y < Lbounds.y) Lbounds.y = node->location.y;
	}
	if (!exists) return Ubounds;
	return Ubounds - Lbounds;
}

void RegionGrammar::DebugPrint() const {
	FString s = TEXT("Graph size: ");
	s.AppendInt(graph.size());
	GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, s);

	// Find max height and width
	location_t bounds{ 0, 0 };
	for (const std::shared_ptr<Node>& node : graph) {
		if (node->location.x > bounds.x) bounds.x = node->location.x;
		if (node->location.y > bounds.y) bounds.y = node->location.y;
	}
	bounds = bounds + location_t{ 1,1 };

	FString b = TEXT("Bounds (h,w): ");
	b.AppendInt(bounds.x);
	b.Append(", ");
	b.AppendInt(bounds.y);
	GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, b);

	// Print
	for (int row = 0; row < bounds.x; row++) {
		FString f = TEXT("");
		for (int col = 0; col < bounds.y; col++) {
			// Find location
			char p = ' ';
			for (const std::shared_ptr<Node>& node : graph)
				if (node->location == location_t{ row, col }) {
					p = static_cast<char>(node->region_label);
					break;
				}
			f.AppendChar(p);
		}
		GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, f);
	}
}

RegionGrammar::graph_t RegionGrammar::ConvertToGraph(const graph_template_t& templ, 
	const RegionGrammar::ConvertToGraphParams& params) {
	RegionGrammar::graph_t out;
	Array2D<std::shared_ptr<Node>> temp_space(templ.size(), templ[0].length());
	bool hit_first_connector = false;
	location_t connectorL_local_location{ 0, 0 };

	int row = 0;
	for (const std::string &line : templ) {
		int col = 0;
		for (const char& c : line) {

			auto label = static_cast<RegionLabel>(c);
			if (label == RegionLabel::none) continue; // Ignore blank spots

			location_t location; // new vertical, horizontal location (local)
			if (params.rotated) location = { col, row };
			else         location = { row, col };

			// Link connectors back to the passed connectors
			if (label == RegionLabel::connector) { 
				temp_space.get(location) = hit_first_connector ? params.connectorR : params.connectorL;
				if (!hit_first_connector) { // This is connectorL
					connectorL_local_location = location;
					hit_first_connector = true;
				}
				continue; // Dont add the connector as a new node, it references the passed nodes. 
			}

			// Add new node
			out.push_back(std::make_shared<Node>(label, location, params.depth));
			std::shared_ptr<Node> added = out.back();

			// Link back references to top & left (previously generated)
			temp_space.get(location) = added;
			std::shared_ptr<Node> top_ref  = temp_space.safe_get(location + E_TOP,  nullptr);
			std::shared_ptr<Node> left_ref = temp_space.safe_get(location + E_LEFT, nullptr);
			added->neighbors[E_TOP]  = top_ref;
			added->neighbors[E_LEFT] = left_ref;
			if (top_ref)  top_ref->neighbors[E_BOTTOM] = added;
			if (left_ref) left_ref->neighbors[E_RIGHT] = added;

			col++;
		}
		row++;
	}

	// Debug message
	if (false) {
		GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, TEXT("Graph loaded (ConvertToGraph): "));
		for (int r = 0; r < temp_space.height; r++) for (int c = 0; c < temp_space.width; c++) {
			auto nullnode = std::make_shared<Node>(Node(RegionLabel::error, { r,c }));
			FString b = TEXT("");
			auto& n = temp_space.safe_get(location_t{ r,c }, nullnode);
			if (n) {
				b.AppendChar(static_cast<char>(n->region_label));
				b.Append(", ");
				b.AppendInt(n->location.x);
				b.Append(", ");
				b.AppendInt(n->location.y);
			}
			GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, b);
		}
	}

	if (params.connectorL) { // Use left connector for global location offset
		// global location offset (temp_space[0,0] location) = cL's global - cL's local
		const location_t& global_location_offset = params.connectorL->location - connectorL_local_location;

		// Apply transformation
		for (std::shared_ptr<Node>&e : out) e->location = e->location + global_location_offset;
	}
	return out;
}