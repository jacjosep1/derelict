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
	
	// Define special characters
	static TCHAR S_ = '_'; // The blank character to use for empty space. 
	static TCHAR SH = 'c'; // The character to use for connecting hallways.
	static TCHAR SR = 'B'; // The character to use for rooms.
	void SetupCharSet(const FString& Set);

	// Define special patterns
	// Empty space
	static pattern_t P_EMPTY_H() {
		return {
			{S_, S_, S_},
			{S_, S_, S_},
			{S_, S_, S_}
		};
	}
	// Horizontal hallway
	static pattern_t P_HALLWAY_H() {
		return {
			{S_, S_, S_},
			{SH, SH, SH},
			{S_, S_, S_}
		};
	}
	// Horizontal hallway left
	static pattern_t P_HALLWAY_HL() {
		return {
			{S_, S_, SR},
			{SH, SH, SR},
			{S_, S_, SR}
		};
	}
	// Horizontal hallway lright
	static pattern_t P_HALLWAY_HR() {
		return {
			{SR, S_, S_},
			{SR, SH, SH},
			{SR, S_, S_}
		};
	}

	// WFC configuration
	constexpr bool			PERIODIC_INPUT	= true;
	constexpr bool			PERIODIC_OUTPUT = false;
	constexpr bool			GROUND			= false; 
	constexpr int32			PATTERNS_SIZE	= 3;
	constexpr unsigned int	SYMMETRY		= 8;

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
	
} // namespace WFC_Interface
