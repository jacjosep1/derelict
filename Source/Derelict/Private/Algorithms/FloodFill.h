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
* @arg empty What to fill for empty regions.
* 
* @returns The input image with only the contiguous region.
*/
template<typename T>
Array2D<T> FloodFill(const Array2D<T>& input, location_t seed,
	const std::function<bool(const T&)> filter, const T& empty) {

	Array2D<char> visited(input.height, input.width, 0);
	Array2D<T> region(input.height, input.width, empty);
	const T &start_color = input.get(seed);

	std::queue<location_t> pixel_queue;
	pixel_queue.push(seed);
	visited.get(seed) = 1;

	while (!pixel_queue.empty()) {
		location_t pixel = pixel_queue.front();
		pixel_queue.pop();
		region.get(pixel) = input.get(pixel);

		for (const location_t& dir : directions) {
			location_t n = pixel + dir;

			if (n.x >= 0 && n.x < input.height
				&& n.y >= 0 && n.y < input.width
				&& visited.get(n) == 0 && filter(input.get(n))) {

				visited.get(n) = 1;
				pixel_queue.push(n);
			}
		}
	}

	return region;
}
