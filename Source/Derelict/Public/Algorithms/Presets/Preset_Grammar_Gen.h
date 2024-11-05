// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <set>

enum class RegionLabel : char {
	none				= ' ',
	error				= '?',
	connector			= '>',

	ship_entrance		= 'e',
	ship_terminal		= 't',
	ship_objective_gen  = 'o',

	ship_large_halls	= 'H',
	ship_medium_halls	= 'h',
	ship_vents			= 'v',

	ship_fillerH		= '_',
	ship_fillerV		= '|',
};

// Structures for grammar rules
typedef std::vector<std::string> graph_template_t;
typedef std::vector<std::pair<std::set<RegionLabel>, std::vector<graph_template_t>>> graph_ruleset_t;

namespace Preset_Grammar_Gen {

	// Define fillers
	namespace { 
		const std::set<RegionLabel> FILLERS_H = {
			RegionLabel::ship_fillerH,
		};
		const std::set<RegionLabel> FILLERS_V = {
			RegionLabel::ship_fillerV,
		};
	}
	
	inline static bool is_filler(const RegionLabel& label) {
		return FILLERS_H.contains(label) || FILLERS_V.contains(label);
	}
	inline static bool is_vertical_filler(const RegionLabel& filler) {
		return FILLERS_V.contains(filler);
	}

	// Access the list of possible generations from a ruleset given some filler.
	namespace { static const std::vector<graph_template_t> EMPTY_SAMPLE_SPACE{}; }
	inline const std::vector<graph_template_t> &access_ruleset(const graph_ruleset_t& ruleset, RegionLabel filler) {
		for (const auto& [filler_set, sample_space] : ruleset)
			if (filler_set.contains(filler)) return sample_space;
		check(false);
		return EMPTY_SAMPLE_SPACE;
	}

	// Default graph initialization
	inline static const graph_template_t START_1 {
		"   e   ",
		"   v   ",
		"   |   ",
		"   h   ",
		"H_hhh_t",
		"   h   ",
		"   |   ",
		"   H   ",
		"   o   ",
	};

	// At least one rule for each label MUST contain a non-filler rule (e.g. <hh<)
	// (options for when max depth is reached)
	// Each rule MUST have two >'s, one to the left of the other. 
	static const graph_ruleset_t rules_fillers {	// Rules for non-max-depth graphs
		// Rules to replace ship_filler1
		{{RegionLabel::ship_fillerH, RegionLabel::ship_fillerV},{
			{" vv  ",
			 ">hh_>",
			 " vv  ",},

			{">HH_>",
			 " HH  ",},

			{"  hhh  ",
		     ">_hhh_>",},

			{"  |  ",
			 ">_v_>",
			 "  |  ",},
		}},
	};
	static const graph_ruleset_t rules_no_fillers{	// Rules for max-depth graphs
		// Rules to replace ship_filler1
		{{RegionLabel::ship_fillerH, RegionLabel::ship_fillerV},{
			{">h>"},
			{">h>"},

			{">vv>"},
		}},
	};

	// Get horizontal filter of vertical filter and vice versa from rules_fillers
	inline RegionLabel alternate_filler(RegionLabel filler) {
		for (const auto& [filler_set, sample_space] : rules_fillers)
			if (filler_set.contains(filler))
				for (const auto& e : filler_set)
					if (e != filler) return e;
		return RegionLabel::error;
	}

}
