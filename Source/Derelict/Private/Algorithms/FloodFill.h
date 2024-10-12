// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Algorithms/WFC_Interface.h"
#include <vector>
#include <queue>

static constexpr location_t directions[4]
	{ {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

/**
* @brief Starting from a seed, return the contiguous region in input defined by filter.
* 
* @arg input The input image
* @arg seed The starting location
* @arg filter A function that takes in the current pixel should return true 
*      iff that pixel should be included in the region.
* 
* @returns A list of pixels in the contiguous region. 
*/
template<typename T>
std::vector<location_t> FloodFill(const TArray2D<T>& input, location_t seed, 
	const std::function<bool(const T&)> filter) {

	TArray2D<char> visited(input.size, 0);
	std::vector<location_t> region;
	T &start_color = input.Get(seed);

	std::queue<location_t> pixel_queue;
	pixel_queue.push(seed);
	visited.Get(seed) = 1;

	while (!pixel_queue.empty()) {
		location_t pixel = pixel_queue.front();
		pixel_queue.pop();
		region.push_back(pixel);

		for (const location_t& dir : directions) {
			location_t n = pixel + dir;

			if (n.x >= 0 && n.x < input.size.x
				&& n.y >= 0 && n.y < input.size.y
				&& visited.Get(n) == 0 && filter(n)) {

				visited.Get(n) = 1;
				pixel_queue.push(n);
			}
		}
	}

	return std::move(region);
}
