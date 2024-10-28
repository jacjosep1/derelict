// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <string>
#include <unordered_map>

enum class RegionLabel : char {
	none				= ' ',
	connector			= '>',
	ship_entrance		= 'e',
	ship_objective_gen  = 'o',
	ship_medium_halls	= 'h',
	ship_filler1		= '_',
};

// Structures for grammar rules
typedef std::vector<std::string> graph_template_t;
typedef std::unordered_map<RegionLabel, std::vector<graph_template_t>> graph_ruleset_t;

namespace Preset_Grammar_Gen {

	// Default graph initialization
	static const graph_template_t START_1 {
		"e_o",
	};

	// At least one rule for each label MUST contain a non-filler rule (e.g. <hh<)
	// (options for when max depth is reached)
	static const graph_ruleset_t rules {
		// Rules to replace ship_filler1
		{RegionLabel::ship_filler1,{
			{{">h>"}},
			{{">h_>"}},
		}},
	};

}
