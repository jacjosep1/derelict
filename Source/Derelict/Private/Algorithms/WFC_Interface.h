// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "Containers/Array.h"

#include "Algorithms/array2D.h"
#include "Algorithms/OverlappingWFC.h"

#include <functional>


struct location_t {
	int32 x{ 0 };
	int32 y{ 0 };

	location_t operator+(const location_t& other) const {
		return location_t{ x + other.x, y + other.y };
	}
};

/**
* Custom 2D array structure
*/
template<typename T>
struct TArray2D {
	location_t size;
	TArray<T> data;

	TArray2D(location_t size_)
		: size(size_), data() {
		data.SetNum(size_.x * size_.y);
	}

	TArray2D(location_t size_, T _value)
		: size(size_), data() {
		data.Init(_value, size_.x * size_.y);
	}

	TArray2D(Array2D<T>& arr_)
		: size{static_cast<int32>(arr_.width), static_cast<int32>(arr_.height)}, data() {
		data.Reserve(arr_.height * arr_.width);
		for (const T i : arr_.data) data.Add(i);
	}

	T &Get(location_t location) {
		return data[location.x + location.y * size.x];
	}

	Array2D<T> ToUnsafe() const {
		Array2D<T> out(static_cast<size_t>(size.x), static_cast<size_t>(size.y));
		for (size_t i = 0; i < data.Num(); i++) out.data[i] = data[i];
		return out;
	}
};

/**
 * Interface for handling WFC
 */
namespace WFC_Interface {

TArray2D<int32> Generate_WFC_Region(const TArray2D<int32>& seed, location_t size);
	
} // namespace WFC_Interface
