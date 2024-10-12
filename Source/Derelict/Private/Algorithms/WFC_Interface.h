// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Algorithms/array2D.h"
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
		: size{arr_.width, arr_.height}, data() {
		data.Reserve(arr_.height * arr_.width);
		for (const T i : arr_.data) data.Add(i);
	}

	T &Get(location_t location) {
		check(location.x < size.x && location.y < size.y);
		check(location.x >= 0 && location.y >= 0);
		return data[location.y + location.x * size.x];
	}

	Array2D<T> ToUnsafe() {
		std::vector<T> new_data{};
		new_data.reserve(data.Num());
		for (const T i : data) new_data.push_back(i);
		return Array2D<T>{size.y, size.x, std::move(new_data)};
	}

	void Loop(const std::function<void(T&, location_t)> func) {
		for (int32 r = 0; r < size.y; r++) for (int32 c = 0; c < size.x; c++)
			func(Get({ c, r }), { c, r }); 
	}
};

/**
 * Interface for handling WFC with UE data structures. 
 */
namespace WFC_Interface {


	
}
