#pragma once

#include "CoreMinimal.h"
#include "Simulation/VitalsSnapshot.h"

/**
 * What the dive watch screen shows: three short lines read from FVitalsSnapshot only.
 *   1  depth            "15.0 m"
 *   2  decompression    "DECO 12.4 h" during the airlock cycle, else "CEIL 7.3 m" above a ceiling, else "NO DECO"
 *   3  day and clock    "D1 06:42" (clock = time of day from GameSeconds)
 */
struct FORTYDAYSUNDER_API FWatchDisplay
{
	static constexpr int32 NumLines = 3;

	/** Freshwater depth for an ambient pressure, using the Buhlmann model's constants; never negative. */
	static double DepthMetres(double AmbientBar);

	static TArray<FString> Lines(const FVitalsSnapshot& Snapshot);
};
