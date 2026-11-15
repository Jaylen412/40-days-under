#include "Simulation/BuhlmannModel.h"

namespace
{
	// ZHL-16C nitrogen: half-time (minutes), a (bar), b (dimensionless).
	constexpr double HalfTimeMin[FBuhlmannModel::NumCompartments] = {
		5.0, 8.0, 12.5, 18.5, 27.0, 38.3, 54.3, 77.0,
		109.0, 146.0, 187.0, 239.0, 305.0, 390.0, 498.0, 635.0 };
	constexpr double CoeffA[FBuhlmannModel::NumCompartments] = {
		1.1696, 1.0, 0.8618, 0.7562, 0.62, 0.5043, 0.441, 0.4,
		0.375, 0.35, 0.3295, 0.3065, 0.2835, 0.261, 0.248, 0.2327 };
	constexpr double CoeffB[FBuhlmannModel::NumCompartments] = {
		0.5578, 0.6514, 0.7222, 0.7825, 0.8126, 0.8434, 0.8693, 0.8910,
		0.9092, 0.9222, 0.9319, 0.9403, 0.9477, 0.9544, 0.9602, 0.9653 };

	constexpr double SeverityRangeBar = 0.6;

	double InspiredN2Bar(double AmbientBar, double FN2)
	{
		return (AmbientBar - FBuhlmannModel::WaterVapourBar) * FN2;
	}
}

double FBuhlmannModel::AmbientBarAtDepth(double DepthM)
{
	return SurfacePressureBar + BarPerMetreFresh * FMath::Max(DepthM, 0.0);
}

void FBuhlmannModel::ResetToSurfaceEquilibrium()
{
	ResetToSaturationAt(0.0, AirFN2);
}

void FBuhlmannModel::ResetToSaturationAt(double DepthM, double FN2)
{
	const double Inspired = InspiredN2Bar(AmbientBarAtDepth(DepthM), FN2);
	for (int32 i = 0; i < NumCompartments; ++i)
	{
		TissueN2Bar[i] = Inspired;
	}
}

void FBuhlmannModel::Advance(double Seconds, double AmbientBar, double FN2)
{
	if (!(Seconds > 0.0))
	{
		return;
	}
	const double Minutes = Seconds / 60.0;
	const double Inspired = InspiredN2Bar(AmbientBar, FN2);
	for (int32 i = 0; i < NumCompartments; ++i)
	{
		const double Fraction = 1.0 - FMath::Pow(2.0, -Minutes / HalfTimeMin[i]);
		TissueN2Bar[i] += (Inspired - TissueN2Bar[i]) * Fraction;
	}
}

double FBuhlmannModel::CeilingBar() const
{
	double Ceiling = -UE_DOUBLE_BIG_NUMBER;
	for (int32 i = 0; i < NumCompartments; ++i)
	{
		Ceiling = FMath::Max(Ceiling, (TissueN2Bar[i] - CoeffA[i]) * CoeffB[i]);
	}
	return Ceiling;
}

bool FBuhlmannModel::IsSafeAt(double AmbientBar) const
{
	return CeilingBar() <= AmbientBar;
}

double FBuhlmannModel::DcsSeverityAt(double AmbientBar) const
{
	return FMath::Clamp((CeilingBar() - AmbientBar) / SeverityRangeBar, 0.0, 1.0);
}

double FBuhlmannModel::DecoAmbientBar(double HoursIntoCycle)
{
	const double Progress = FMath::Clamp(HoursIntoCycle / DecoCycleHours, 0.0, 1.0);
	return AmbientBarAtDepth(LabDepthM * (1.0 - Progress));
}
