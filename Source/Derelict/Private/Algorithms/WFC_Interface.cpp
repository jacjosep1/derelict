// Fill out your copyright notice in the Description page of Project Settings.


#include "Algorithms/WFC_Interface.h"
#include "Algorithms/FloodFill.h"

template <typename TPreset>
Array2D<TCHAR> WFC_Interface<TPreset>::ReadImage_CSV(UDataTable* Data, bool DebugString) const {
    if (!Data) {
        UE_LOG(LogTemp, Error, TEXT("DataTable null."));
        return Array2D<TCHAR>();
    }

    // Get array of strings from data table.  
    TArray<FString> StrList;

    TArray<FImageCSV_Row*> AllRows;
    Data->GetAllRows<FImageCSV_Row>(TEXT("Loading seed CSV for WFC"), AllRows);
    for (const FImageCSV_Row* Row : AllRows)
        if (Row) StrList.Add(Row->V);

    // Convert to a custom 2d array
    TArray<TCHAR> data;
    int32 InputX = 0;
    int32 InputY = StrList.Num();
    Array2D<TCHAR> ret;

    for (int32 y = 0; y < InputY; y++) {
        const auto& StrLine = StrList[y];
        if (InputX != 0) {
            check(StrLine.Len() == InputX);
        }
        else {
            InputX = StrLine.Len();
            ret.Init({ InputX, InputY }, '0');
        }
        if (DebugString) GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, StrLine);

        for (int32 x = 0; x < InputX; x++)
            ret.get({ x, y }) = StrLine[x];
    }
    return ret;
}

template <typename TPreset>
Array2D<TCHAR> WFC_Interface<TPreset>::Generate_WFC_Region(const Array2D<TCHAR>& seed, location_t size, std::vector<EDir> exits_in) {

    // Make room for border
    auto crop_amt = PATTERNS_SIZE - 1;
    size.x += crop_amt * 2;
    size.y += crop_amt * 2;

    // Config
	OverlappingWFCOptions options;
	options.periodic_input =    PERIODIC_INPUT;
	options.periodic_output =   PERIODIC_OUTPUT;
	options.out_height =        size.x;
	options.out_width =         size.y;
	options.symmetry =          SYMMETRY;
	options.ground =            GROUND;
	options.pattern_size =      PATTERNS_SIZE;

	for (size_t i = 0; i < FAIL_COUNT; i++) { // TODO - make it so this never fails. 
		OverlappingWFC<TCHAR> wfc(seed, options, FMath::RandRange(1, MAX_INT32));

        // Make exits at midpoints
        std::vector<ExitLocation> exits;
        exits.reserve(exits_in.size());
        for (const auto& e : exits_in) {
            constexpr int32 DIV = 3;
            for (int32 v = 1; v < DIV; v++)
                exits.push_back({ e, 
                    (e == E_TOP || e == E_BOTTOM) ?
                    Linspace(size.x, DIV, v) / PATTERNS_SIZE :
                    Linspace(size.y, DIV, v) / PATTERNS_SIZE
                });
        }

        PreCollapseBorder(wfc, exits);
		auto out = wfc.run();

        if (out.has_value()) {

            // Only select contiguous region from an exit. Assume center is filled. 
            check(exits.size() > 0);
            auto out_cont = SelectByColor(*out, exits[0].offset_physical(size, true), TPreset::S_, true);

            // Verify there is a path from exit to entrance
            bool invalidate = false;
            for (const auto& exit : exits)
                if (out_cont.get(exit.offset_physical(size, true)) == TPreset::S_) { // Check for blank spot centered at the exits
                    invalidate = true;
                    if (DEBUG_MESSAGES) GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, TEXT("Invalid exit path"));
                }
            if (!invalidate) {
                // Crop to ~border
                out_cont = out_cont.center_crop(crop_amt);
                return out_cont;
            }
        }
        else {
            if (DEBUG_MESSAGES) GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, TEXT("WFC constrained too much"));
        }
	}
    GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, TEXT("Failed WFC too many times"));
	return Array2D<TCHAR>(location_t{0, 0});
}

template <typename TPreset>
Array2D<TCHAR> WFC_Interface<TPreset>::SelectByColor(const Array2D<TCHAR>& region, location_t seed, TCHAR color, bool null) {
    std::function<bool(const TCHAR&)> selector;
    if (null) selector = [&](const TCHAR& t) -> bool { return (t != color); };
    else      selector = [&](const TCHAR& t) -> bool { return (t == color); };
	return FloodFill<TCHAR>(region, seed, selector, TPreset::S_);
}

template <typename TPreset>
void WFC_Interface<TPreset>::PreCollapsePoints(OverlappingWFC<TCHAR>& wfc, const std::vector<location_t>& points, const pattern_t &pattern) {
    check(pattern.size() == PATTERNS_SIZE);
    check(pattern[0].size() == PATTERNS_SIZE);
    for (const auto& point : points) {
        Array2D<TCHAR> fill({ PATTERNS_SIZE, PATTERNS_SIZE }, pattern);
        bool r = wfc.set_pattern(fill, point.x, point.y);

        if (!r) if (DEBUG_MESSAGES)  GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, TEXT("Failed setpattern"));
    }
}

template <typename TPreset>
void WFC_Interface<TPreset>::PreCollapseBorder(OverlappingWFC<TCHAR>& wfc, const std::vector<ExitLocation> &exits) {
    location_t size = { wfc.get_options().out_height, wfc.get_options().out_width };

    const int32 subgrid_x = size.x / PATTERNS_SIZE;
    const int32 subgrid_y = size.y / PATTERNS_SIZE;

    auto side_fill = [&](int32 max_index, const EDir side) {
        std::vector<location_t> Epoints_of_interest;
        std::vector<location_t> Hpoints_of_interest;
        Epoints_of_interest.reserve(max_index);
        Hpoints_of_interest.reserve(exits.size());

        // Empty border
        for (int32 j = 0; j < max_index; j++) {
            bool skip = false;
            for (const auto& exit : exits)
                if (exit.side == side && exit.offset == j) skip = true;
            if (!skip) Epoints_of_interest.push_back(SIDE_TO_PHYSICAL(side, size, j));
        }
        PreCollapsePoints(wfc, Epoints_of_interest, TPreset::P_EMPTY_H);

        // Exit hallways
        for (const ExitLocation& exit : exits) {
            if (exit.side == side)
                Hpoints_of_interest.push_back(exit.offset_physical(size));
        }
        PreCollapsePoints(wfc, Hpoints_of_interest, *TPreset::EXIT_PATTERNS[side]);
    };

    side_fill(subgrid_x, E_LEFT);
    side_fill(subgrid_y, E_TOP);
    side_fill(subgrid_x, E_RIGHT);
    side_fill(subgrid_y, E_BOTTOM);
}

template class WFC_Interface<PRESET_MediumHalls>;