// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include <optional>
#include <random>

#include "Algorithms/WFC.h"
#include "Algorithms/array2D.h"
#include "Algorithms/Wave.h"
#include "Algorithms/Propagator.h"

/**
 * Class containing the generic WFC algorithm.
 */
class WFC {
private:
    /**
     * The random number generator.
     */
    std::minstd_rand gen;

    /**
     * The distribution of the patterns as given in input.
     */
    const std::vector<double> patterns_frequencies;

    /**
     * The wave, indicating which patterns can be put in which cell.
     */
    Wave wave;

    /**
     * The number of distinct patterns.
     */
    const size_t nb_patterns;

    /**
     * The propagator, used to propagate the information in the wave.
     */
    Propagator propagator;

    /**
     * Transform the wave to a valid output (a 2d array of patterns that aren't in
     * contradiction). This function should be used only when all cell of the wave
     * are defined.
     */
    Array2D<unsigned> wave_to_output() const noexcept;

public:
    /**
     * Basic constructor initializing the algorithm.
     */
    WFC(bool periodic_output, int seed, std::vector<double> patterns_frequencies,
        Propagator::PropagatorState propagator, unsigned wave_height,
        unsigned wave_width)
        noexcept;

    /**
     * Run the algorithm, and return a result if it succeeded.
     */
    std::optional<Array2D<unsigned>> run() noexcept;

    /**
     * Return value of observe.
     */
    enum ObserveStatus {
        success,    // WFC has finished and has succeeded.
        failure,    // WFC has finished and failed.
        to_continue // WFC isn't finished.
    };

    /**
     * Define the value of the cell with lowest entropy.
     */
    ObserveStatus observe() noexcept;

    /**
     * Propagate the information of the wave.
     */
    void propagate() noexcept { propagator.propagate(wave); }

    /**
     * Remove pattern from cell (i,j).
     */
    void remove_wave_pattern(unsigned i, unsigned j, unsigned pattern) noexcept {
        if (wave.get(i, j, pattern)) {
            wave.set(i, j, pattern, false);
            propagator.add_to_propagator(i, j, pattern);
        }
    }
};
