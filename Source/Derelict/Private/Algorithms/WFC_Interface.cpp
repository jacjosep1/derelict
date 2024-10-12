// Fill out your copyright notice in the Description page of Project Settings.


#include "Algorithms/WFC_Interface.h"

TArray2D<TCHAR> WFC_Interface::Generate_WFC_Region(const TArray2D<TCHAR>& seed, location_t size) {

	OverlappingWFCOptions options;
	options.periodic_input = true;
	options.periodic_output = false;
	options.out_height = size.y;
	options.out_width = size.x;
	options.symmetry = 4;
	options.ground = false;
	options.pattern_size = 3;

	Array2D<TCHAR> seed_local = seed.ToUnsafe();

	for (size_t i = 0; i < 100; i++) {
		OverlappingWFC<TCHAR> wfc(seed_local, options, FMath::FRand());
		auto out = wfc.run();
		if (out.has_value()) return TArray2D(*out);
	}
	check(false);
	return TArray2D<TCHAR>({0, 0});
}
