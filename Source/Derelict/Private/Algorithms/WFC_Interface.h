// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "Containers/Array.h"
#include "Engine/DataTable.h"
#include "Structs/CommonStructs.h"

#include "Algorithms/array2D.h"
#include "Algorithms/OverlappingWFC.h"
#include "Algorithms/Presets/Preset_WFC_Gen.h"

#include <functional>

/**
 * Interface for handling WFC
 */
template <typename TPreset> class WFC_Interface {
	static_assert(is_preset_wfc_gen<TPreset>::value, "Preset should be a Preset_WFC_Gen");

	static constexpr int32 MAX_INT32 = 0x7FFFFFFF;
	static constexpr bool DEBUG_MESSAGES{ false };

	// WFC configuration
	static constexpr bool			PERIODIC_INPUT = false;
	static constexpr bool			PERIODIC_OUTPUT = false;
	static constexpr bool			GROUND = false;
	static constexpr int32			PATTERNS_SIZE = 3;
	static constexpr unsigned int	SYMMETRY = 8;

	// The max number of times to fail WFC before exiting. 
	static constexpr size_t FAIL_COUNT = 300;

public:
	// Function to convert from side offsets (in units of pattern size) to physical location
	static inline location_t SIDE_TO_PHYSICAL(EDir side, location_t size, int32 j) {
		if		(side == E_TOP)		return { 0,							j * PATTERNS_SIZE };
		else if (side == E_BOTTOM)	return { size.x - PATTERNS_SIZE,	j * PATTERNS_SIZE };
		else if (side == E_LEFT)	return { j * PATTERNS_SIZE,			0 };
		else if (side == E_RIGHT)	return { j * PATTERNS_SIZE,			size.y - PATTERNS_SIZE };
		else {
			check(false);
			return { 0, 0 };  // Should not reach. 
		}
	}

	// Wrapper for exit location config
	struct ExitLocation {
		EDir side{ E_LEFT };
		int32 offset{ 0 };

		inline bool operator==(const ExitLocation& other) const {
			return side == other.side && offset == other.offset;
		}

		// If centering is false, returns the upper left corner. Otherwise, returns the center. 
		inline location_t offset_physical(location_t size, bool centered = false) const {
			location_t centering = centered ? location_t{ PATTERNS_SIZE / 2, PATTERNS_SIZE / 2 } : location_t{ 0, 0 };
			return SIDE_TO_PHYSICAL(side, size, offset) + centering;
		}
	};

	// Struct for storing tile properties in addition to label. 
	struct TileProperties {
		TCHAR label;
		int32 room_index;		// -1 if not a valid room
		bool  turret_level;		// indicates if turrets should be present

		TileProperties() : room_index(-1), turret_level(false) {}
	};

	// Wrapper for WFC Gen output
	struct Generate_WFC_Region_Output {
		Array2D<TCHAR> raw_labels;
		Array2D<TileProperties> property_grid;

		static inline Generate_WFC_Region_Output dummy() {
			return { Array2D<TCHAR>(location_t{ 0, 0 }), Array2D<TileProperties>(location_t{ 0, 0 }) };
		}
	};

	// Read a data table representing an image and convert into a 2D array of labels. Optionally prints the data. 
	Array2D<TCHAR> ReadImage_CSV(UDataTable* Data, bool DebugString = false) const;

	// Generate a region of a certain size using WFC and a seed determining pattern rules. 
	Generate_WFC_Region_Output Generate_WFC_Region(const Array2D<TCHAR>& seed, location_t size, std::vector<EDir> exit,
		RegionLabel region_label = RegionLabel::ship_vents);

	// Starting from the seed, remove everything except except the locally contiguous region.
	// If null=false, select adjacent pixels of the specified color.
	// If null=true, select adjacent pixels that are NOT the specified color.
	// Convention: removed parts are replaced by null_space. 
	Array2D<TCHAR> SelectByColor(const Array2D<TCHAR>& region, location_t seed, TCHAR color, bool null);

	// Precollapse a group of points to a specific pattern before running the WFC. 
	void PreCollapsePoints(OverlappingWFC<TCHAR>& wfc, const std::vector<location_t>& points, const pattern_t& pattern);

	// Generate a border with specified exit points. Useful to contain a generated region and provide an interface to other regions.
	// Cropping may be necessary after doing this by pattern_size - 1. 
	void PreCollapseBorder(OverlappingWFC<TCHAR>& wfc, const std::vector<ExitLocation>& exits);

	// Utility functions

	static int32 Linspace(int32 length, int32 div, int32 pos) {
		return length / div * pos;
	}

	static std::vector<size_t> PickUniqueRandomInts(size_t N, size_t max, size_t min = 0);

}; // namespace WFC_Interface

struct Region_Properties {
	float turret_room_density;
	int32 turret_spacing;
};

template<typename Gen> struct Preset_WFC_Specification {

	int32 scale{ 1 };
	WFC_Interface<Gen> generator;
	Array2D<TCHAR> seed;

	Preset_WFC_Specification() = default;
	Preset_WFC_Specification(int32 scale_, WFC_Interface<Gen> generator_)
		: scale{ scale_ }, generator{ generator_ } {
	}
};

using SPECIFICATION_VARIANT = std::variant<
	Preset_WFC_Specification<PRESET_MediumHalls>
>;

// Regions to WFC Gens
struct spec_wrapper {
	SPECIFICATION_VARIANT spec;
	Region_Properties properties;
};
static std::unordered_map<RegionLabel, spec_wrapper> WFC_SPECIFICATIONS {
	{ RegionLabel::ship_entrance,      {Preset_WFC_Specification<PRESET_MediumHalls>(1, WFC_Interface<PRESET_MediumHalls>()), Region_Properties{
		0.5, 3
		}}},
	{ RegionLabel::ship_terminal,      {Preset_WFC_Specification<PRESET_MediumHalls>(1, WFC_Interface<PRESET_MediumHalls>()), Region_Properties{
		0.5, 3
		}}},
	{ RegionLabel::ship_vents,         {Preset_WFC_Specification<PRESET_MediumHalls>(1, WFC_Interface<PRESET_MediumHalls>()), Region_Properties{
		0.5, 3
		}}},
	{ RegionLabel::ship_objective_gen, {Preset_WFC_Specification<PRESET_MediumHalls>(1, WFC_Interface<PRESET_MediumHalls>()), Region_Properties{
		0.5, 3
		}}},
	{ RegionLabel::ship_medium_halls,  {Preset_WFC_Specification<PRESET_MediumHalls>(1, WFC_Interface<PRESET_MediumHalls>()), Region_Properties{
		0.5, 3
		}}},
	{ RegionLabel::ship_large_halls,   {Preset_WFC_Specification<PRESET_MediumHalls>(1, WFC_Interface<PRESET_MediumHalls>()), Region_Properties{
		0.5, 3
		}}},
};




