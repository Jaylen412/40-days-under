#include "Simulation/AirSupplyModel.h"

namespace
{
	constexpr double BaselineCo2Pct = 0.04;
	constexpr double BaselineO2Pct = 20.9;
}

void FAirSupplyModel::Advance(double Seconds)
{
	if (!(Seconds > 0.0) || !FMath::IsFinite(Seconds))
	{
		return;
	}
	double Hours = Seconds / 3600.0;

	// Pump running: burn fuel and scrub, up to the moment the tank runs dry.
	if (FuelL > 0.0)
	{
		const double HoursToDry = FuelL / BurnLPerHour;
		const double RunHours = FMath::Min(Hours, HoursToDry);
		FuelL = (Hours >= HoursToDry) ? 0.0 : FuelL - Hours * BurnLPerHour;
		Co2Pct = FMath::Max(BaselineCo2Pct, Co2Pct - Co2ScrubPctPerHour * RunHours);
		O2Pct = FMath::Min(BaselineO2Pct, O2Pct + O2RecoverPctPerHour * RunHours);
		Hours -= RunHours;
	}

	// Pump dead for the rest of the step.
	if (Hours > 0.0)
	{
		Co2Pct = FMath::Min(100.0, Co2Pct + Co2RisePctPerHour * Hours);
		O2Pct = FMath::Max(0.0, O2Pct - O2FallPctPerHour * Hours);
	}
}

double FAirSupplyModel::AddFuel(double Litres)
{
	if (!(Litres > 0.0))
	{
		return 0.0;
	}
	const double Accepted = FMath::Min(Litres, TankCapacityL - FuelL);
	FuelL += Accepted;
	return Accepted;
}

double FAirSupplyModel::FuelFraction() const
{
	return FuelL / TankCapacityL;
}

bool FAirSupplyModel::IsPumpRunning() const
{
	return FuelL > 0.0;
}

bool FAirSupplyModel::IsSputtering() const
{
	return IsPumpRunning() && FuelFraction() < SputterFraction;
}

EAirStage FAirSupplyModel::Stage() const
{
	if (Co2Pct >= 10.0 || O2Pct <= 10.0)
	{
		return EAirStage::Dead;
	}
	if (Co2Pct >= 8.0)
	{
		return EAirStage::Critical;
	}
	if (Co2Pct >= 5.0)
	{
		return EAirStage::Hypoxic;
	}
	if (Co2Pct >= 3.0)
	{
		return EAirStage::Impaired;
	}
	return EAirStage::Nominal;
}
