#pragma once

#include "CoreMinimal.h"

/** Bühlmann ZHL-16C nitrogen tissue model. Plain struct, no UObject. Pressures in bar. */
struct FBuhlmannModel
{
	static constexpr int32 NumCompartments = 16;
	static constexpr double SurfacePressureBar = 1.01325;
	static constexpr double BarPerMetreFresh = 0.0980665;
	static constexpr double WaterVapourBar = 0.0627;
	static constexpr double AirFN2 = 0.79;
	static constexpr double DecoGasFN2 = 0.60;
	static constexpr double LabDepthM = 15.0;
	static constexpr double DecoCycleHours = 17.0;

	double TissueN2Bar[NumCompartments];

	/** Starts at surface equilibrium on air. */
	FBuhlmannModel() { ResetToSurfaceEquilibrium(); }

	static double AmbientBarAtDepth(double DepthM);
	void ResetToSurfaceEquilibrium();
	void ResetToSaturationAt(double DepthM, double FN2);
	void Advance(double Seconds, double AmbientBar, double FN2);   // exact exponential, any step size
	double CeilingBar() const;                 // max over i of (P_i - a_i) * b_i
	bool IsSafeAt(double AmbientBar) const;    // CeilingBar() <= AmbientBar
	double DcsSeverityAt(double AmbientBar) const; // clamp((Ceiling - Ambient) / 0.6, 0, 1)
	static double DecoAmbientBar(double HoursIntoCycle); // linear 15 m -> 0 m over DecoCycleHours, clamped
};
