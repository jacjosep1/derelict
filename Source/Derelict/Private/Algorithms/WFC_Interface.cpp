// Fill out your copyright notice in the Description page of Project Settings.


#include "Algorithms/WFC_Interface.h"
#include "Algorithms/FloodFill.h"

void WFC_Interface::SetupCharSet(const FString& Set) {
    S_ = Set[0];
    SH = Set[1];
    SR = Set[2];
}

Array2D<TCHAR> WFC_Interface::ReadImage_CSV(UDataTable* Data, bool DebugString) {
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

Array2D<TCHAR> WFC_Interface::Generate_WFC_Region(const Array2D<TCHAR>& seed, location_t size) {
	OverlappingWFCOptions options;
	options.periodic_input =    PERIODIC_INPUT;
	options.periodic_output =   PERIODIC_OUTPUT;
	options.out_height =        size.x;
	options.out_width =         size.y;
	options.symmetry =          SYMMETRY;
	options.ground =            GROUND;
	options.pattern_size =      PATTERNS_SIZE;

	for (size_t i = 0; i < FAIL_COUNT; i++) { // TODO - make it so this never fails. 
        //GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, FString::FromInt(options.pattern_size));
		OverlappingWFC<TCHAR> wfc(seed, options, FMath::RandRange(1, 100000));

        // Fill in border so there aren't dead ends
        // TODO - move this and generalize it. 
        std::vector<location_t> Epoints_of_interest;
        const int32 subgrid_x = size.x / PATTERNS_SIZE;
        const int32 subgrid_y = size.y / PATTERNS_SIZE;
        Epoints_of_interest.reserve(2*subgrid_x + 2* subgrid_y);

        const int32 exit_x = 7 * PATTERNS_SIZE;
        for (int32 j = 0; j < subgrid_x; j++) 
            if (j*PATTERNS_SIZE != exit_x) 
                Epoints_of_interest.push_back({ j * PATTERNS_SIZE, 0 });
        for (int32 j = 0; j < subgrid_y; j++) Epoints_of_interest.push_back({ 0, j * PATTERNS_SIZE });
        for (int32 j = 0; j < subgrid_x; j++)
            if (j * PATTERNS_SIZE != exit_x)
                Epoints_of_interest.push_back({ j * PATTERNS_SIZE, size.y - PATTERNS_SIZE });
        for (int32 j = 0; j < subgrid_y; j++) Epoints_of_interest.push_back({ size.x - PATTERNS_SIZE, j * PATTERNS_SIZE });

        PreCollapsePoints(wfc, Epoints_of_interest, P_EMPTY_H());

        // Fill in edge hallways. 
        // TODO - vertical. 
        PreCollapsePoints(wfc, { {exit_x, 0} },                      P_HALLWAY_H());
        PreCollapsePoints(wfc, { {exit_x, size.y - PATTERNS_SIZE} }, P_HALLWAY_H());

		auto out = wfc.run();

        if (out.has_value()) {
            // Only select contiguous region.
            auto out_cont = SelectByColor(*out, { exit_x + 1, 0 }, S_, true);

            // Verify there is a path from exit to entrance
            if (out_cont.get({exit_x + 1, size.y - PATTERNS_SIZE}) == SH)
                return out_cont;
        }
	}
    GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, TEXT("Skipped generation"));
	return Array2D<TCHAR>(location_t{0, 0});
}

Array2D<TCHAR> WFC_Interface::SelectByColor(const Array2D<TCHAR>& region, location_t seed, TCHAR color, bool null) {
    std::function<bool(const TCHAR&)> selector;
    if (null) selector = [&](const TCHAR& t) -> bool { return (t != color); };
    else      selector = [&](const TCHAR& t) -> bool { return (t == color); };
	return FloodFill<TCHAR>(region, seed, selector, S_);
}

void WFC_Interface::PreCollapsePoints(OverlappingWFC<TCHAR>& wfc, const std::vector<location_t>& points, const pattern_t &pattern) {
    check(pattern.size() == PATTERNS_SIZE);
    check(pattern[0].size() == PATTERNS_SIZE);
    for (const auto& point : points) {
        Array2D<TCHAR> fill({ PATTERNS_SIZE, PATTERNS_SIZE }, pattern);
        bool r = wfc.set_pattern(fill, point.x, point.y);

        if (!r) GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Green, TEXT("Failed setpattern"));
    }
}
