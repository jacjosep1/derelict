// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Algorithms/Wave.h"
#include "Algorithms/array3D.h"

#include <array>
#include <vector>

constexpr int directions_x[4] = { 0, -1, 1, 0 };
constexpr int directions_y[4] = { -1, 0, 0, 1 };

constexpr unsigned get_opposite_direction(unsigned direction) noexcept {
	return 3 - direction;
}


class Propagator {
public:
    using PropagatorState = std::vector<std::array<std::vector<unsigned>, 4>>;

private:

    const std::size_t patterns_size;
    PropagatorState propagator_state;
    const unsigned wave_width;
    const unsigned wave_height;
    const bool periodic_output;
    std::vector<std::tuple<unsigned, unsigned, unsigned>> propagating;
    Array3D<std::array<int, 4>> compatible;

    void init_compatible() noexcept {
        std::array<int, 4> value;

        for (unsigned y = 0; y < wave_height; y++) {
            for (unsigned x = 0; x < wave_width; x++) {
                for (unsigned pattern = 0; pattern < patterns_size; pattern++) {
                    for (int direction = 0; direction < 4; direction++) {
                        value[direction] =
                            static_cast<unsigned>(propagator_state[pattern][get_opposite_direction(direction)]
                                .size());
                    }
                    compatible.get(y, x, pattern) = value;
                }
            }
        }
    }

public:

    Propagator(unsigned wave_height, unsigned wave_width, bool periodic_output,
        PropagatorState propagator_state) noexcept
        : patterns_size(propagator_state.size()),
        propagator_state(propagator_state), wave_width(wave_width),
        wave_height(wave_height), periodic_output(periodic_output),
        compatible(wave_height, wave_width, patterns_size) {
        init_compatible();
    }

    void add_to_propagator(unsigned y, unsigned x, unsigned pattern) noexcept {
        std::array<int, 4> temp = {};
        compatible.get(y, x, pattern) = temp;
        propagating.emplace_back(y, x, pattern);
    }

    void propagate(Wave& wave) noexcept {
        while (propagating.size() != 0) {
            unsigned y1, x1, pattern;
            std::tie(y1, x1, pattern) = propagating.back();
            propagating.pop_back();

            for (unsigned direction = 0; direction < 4; direction++) {
                int dx = directions_x[direction];
                int dy = directions_y[direction];
                int x2, y2;
                if (periodic_output) {
                    x2 = ((int)x1 + dx + (int)wave.width) % wave.width;
                    y2 = ((int)y1 + dy + (int)wave.height) % wave.height;
                }
                else {
                    x2 = x1 + dx;
                    y2 = y1 + dy;
                    if (x2 < 0 || x2 >= (int)wave.width) {
                        continue;
                    }
                    if (y2 < 0 || y2 >= (int)wave.height) {
                        continue;
                    }
                }

                unsigned i2 = x2 + y2 * wave.width;
                const std::vector<unsigned>& patterns =
                    propagator_state[pattern][direction];

                for (auto it = patterns.begin(), it_end = patterns.end(); it < it_end;
                    ++it) {

                    std::array<int, 4>& value = compatible.get(y2, x2, *it);
                    value[direction]--;
                    if (value[direction] == 0) {
                        add_to_propagator(y2, x2, *it);
                        wave.set(i2, *it, false);
                    }
                }
            }
        }
    }
};
