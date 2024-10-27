// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <functional>

typedef std::vector<std::vector<TCHAR>> pattern_t;

// Enum/struct for defining where the exit points on the generated image should be.
enum EExitLocation : char {
	E_TOP, E_BOTTOM, E_LEFT, E_RIGHT
};

template<TCHAR _S_=0, TCHAR _SH=0, TCHAR _SR=0, TCHAR... _SC>
struct Preset_WFC_Gen
{
	// Define special characters
	static constexpr TCHAR S_{ _S_ };									// The blank character to use for empty space. 
	static constexpr TCHAR SH{ _SH };									// The character to use for connecting hallways.
	static constexpr TCHAR SR{ _SR };									// The character to use for rooms.
	static constexpr std::array<TCHAR, sizeof...(_SC)> SC { _SC... };	// Additional custom characters

	// Define special patterns
	// Empty space
	inline static const pattern_t P_EMPTY_H {
		{S_, S_, S_},
		{S_, S_, S_},
		{S_, S_, S_}
	};
	// Horizontal hallway
	inline static const pattern_t P_HALLWAY_H {
		{S_, S_, S_},
		{SH, SH, SH},
		{S_, S_, S_}
	};
	// Vertical hallway
	inline static const pattern_t P_HALLWAY_V {
		{S_, SH, S_},
		{S_, SH, S_},
		{S_, SH, S_}
	};

	// Defines patterns for exits on different orientations of edges. 
	inline static constexpr const pattern_t* EXIT_PATTERNS[] {
		& P_HALLWAY_V,
		& P_HALLWAY_V,
		& P_HALLWAY_H,
		& P_HALLWAY_H,
	};

};

template <typename T> struct is_preset_wfc_gen : std::false_type {};
template <TCHAR... P> struct is_preset_wfc_gen<Preset_WFC_Gen<P...>> : std::true_type {};

typedef Preset_WFC_Gen<'_', 'c', 'B'> PRESET_MediumHalls;
