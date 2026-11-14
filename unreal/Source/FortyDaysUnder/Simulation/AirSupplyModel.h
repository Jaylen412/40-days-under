#pragma once

#include "CoreMinimal.h"
#include "Simulation/VitalsSnapshot.h"

/** Diesel-driven air pump and scrubber. Plain struct, no UObject. */
struct FAirSupplyModel
{
	static constexpr double TankCapacityL = 2000.0;
	static constexpr double BurnLPerHour = 2.0;        // dry at day 41.67 from full
	static constexpr double SputterFraction = 0.15;
	static constexpr double Co2RisePctPerHour = 0.5;   // pump dead
	static constexpr double O2FallPctPerHour = 0.5;    // pump dead
	static constexpr double Co2ScrubPctPerHour = 2.0;  // pump running, down to 0.04
	static constexpr double O2RecoverPctPerHour = 2.4; // pump running, up to 20.9

	double FuelL = TankCapacityL;
	double Co2Pct = 0.04;
	double O2Pct = 20.9;

	void Advance(double Seconds);
	double AddFuel(double Litres);          // returns litres accepted; clamps to capacity; negative input accepted as 0
	double FuelFraction() const;
	bool IsPumpRunning() const;             // FuelL > 0
	bool IsSputtering() const;              // running and FuelFraction() < SputterFraction
	EAirStage Stage() const;
};
