// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "Containers/Array.h"
#include "Engine/DataTable.h"
#include "Structs/CommonStructs.h"

#include "Algorithms/array2D.h"
#include "Algorithms/OverlappingWFC.h"

#include <functional>

typedef std::vector<std::vector<TCHAR>> pattern_t;

/**
 * Interface for handling WFC
 */
namespace WFC_Interface {

	constexpr int32 MAX_INT32 = 0x7FFFFFFF;

	// WFC configuration
	constexpr bool			PERIODIC_INPUT = false;
	constexpr bool			PERIODIC_OUTPUT = false;
	constexpr bool			GROUND = false;
	constexpr int32			PATTERNS_SIZE = 3;
	constexpr unsigned int	SYMMETRY = 8;

	// Define special characters
	static TCHAR S_ = '_'; // The blank character to use for empty space. 
	static TCHAR SH = 'c'; // The character to use for connecting hallways.
	static TCHAR SR = 'B'; // The character to use for rooms.

	// Set should contain initialization for [Blank, Hallway, Room] characters. 
	void SetupCharSet(const FString& Set);

	// Define special patterns
	// Empty space
	static inline pattern_t P_EMPTY_H() {
		return {
			{S_, S_, S_},
			{S_, S_, S_},
			{S_, S_, S_}
		};
	}
	// Horizontal hallway
	static inline pattern_t P_HALLWAY_H() {
		return {
			{S_, S_, S_},
			{SH, SH, SH},
			{S_, S_, S_}
		};
	}
	// Vertical hallway
	static inline pattern_t P_HALLWAY_V() {
		return {
			{S_, SH, S_},
			{S_, SH, S_},
			{S_, SH, S_}
		};
	}

	// Enum/struct for defining where the exit points on the generated image should be.
	enum EExitLocation {
		E_TOP, E_BOTTOM, E_LEFT, E_RIGHT
	};

	// Defines patterns for exits on different orientations of edges. 
	static std::unordered_map<EExitLocation, std::function<pattern_t(void)>> EXIT_PATTERNS = {
		{E_LEFT, P_HALLWAY_H},
		{E_RIGHT, P_HALLWAY_H},
		{E_TOP, P_HALLWAY_V},
		{E_BOTTOM, P_HALLWAY_V}
	};

	// Function to convert from side offsets (in units of pattern size) to physical location
	inline location_t SIDE_TO_PHYSICAL(EExitLocation side, location_t size, int32 j) {
		switch (side) {
		case WFC_Interface::E_TOP:
			return { 0, j * PATTERNS_SIZE };
		case WFC_Interface::E_BOTTOM:
			return { size.x - PATTERNS_SIZE, j * PATTERNS_SIZE };
		case WFC_Interface::E_LEFT:
			return { j * PATTERNS_SIZE, 0 };
		case WFC_Interface::E_RIGHT:
			return { j * PATTERNS_SIZE, size.y - PATTERNS_SIZE };
		default:
			return { 0, 0 };  // Should not reach. 
		}
	}

	// Wrapper for exit location config
	struct ExitLocation {
		EExitLocation side{E_LEFT};
		int32 offset{0};

		inline bool operator==(const ExitLocation& other) const {
			return side == other.side && offset == other.offset;
		}

		// If centering is false, returns the upper left corner. Otherwise, returns the center. 
		inline location_t offset_physical(location_t size, bool centered=false) const {
			location_t centering = centered ? location_t{PATTERNS_SIZE/2, PATTERNS_SIZE/2} : location_t{0, 0};
			return SIDE_TO_PHYSICAL(side, size, offset) + centering;
		}
	};

	// The max number of times to fail WFC before exiting. 
	constexpr size_t FAIL_COUNT = 20;

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
	void PreCollapsePoints(OverlappingWFC<TCHAR>& wfc, const std::vector<location_t> &points, const pattern_t &pattern);

	// Generate a border with specified exit points. Useful to contain a generated region and provide an interface to other regions.
	// Cropping may be necessary after doing this by pattern_size - 1. 
	void PreCollapseBorder(OverlappingWFC<TCHAR>& wfc, const std::vector<ExitLocation> &exits);
	
} // namespace WFC_Interface
