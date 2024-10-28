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

	// WFC configuration
	static constexpr bool			PERIODIC_INPUT = false;
	static constexpr bool			PERIODIC_OUTPUT = false;
	static constexpr bool			GROUND = false;
	static constexpr int32			PATTERNS_SIZE = 3;
	static constexpr unsigned int	SYMMETRY = 8;

	// The max number of times to fail WFC before exiting. 
	static constexpr size_t FAIL_COUNT = 20;

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

	// Read a data table representing an image and convert into a 2D array of labels. Optionally prints the data. 
	Array2D<TCHAR> ReadImage_CSV(UDataTable* Data, bool DebugString = false);

	// Generate a region of a certain size using WFC and a seed determining pattern rules. 
	Array2D<TCHAR> Generate_WFC_Region(const Array2D<TCHAR>& seed, location_t size);

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

}; // namespace WFC_Interface

