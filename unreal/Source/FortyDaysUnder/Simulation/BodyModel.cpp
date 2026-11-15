#include "Simulation/BodyModel.h"

namespace
{
	constexpr double SecondsPerDay = 86400.0;
	constexpr double DehydratedHealthPerSecond = 100.0 / (3.0 * SecondsPerDay);
	constexpr double StarvingHealthPerSecond = 100.0 / (21.0 * SecondsPerDay);
	constexpr double CriticalAirHealthPerSecond = 100.0 / 3600.0;
	constexpr double DcsHealthPerSecondAtFullSeverity = 0.5;

	/** Drains Reserve at Rate for Seconds; returns the seconds spent at zero. */
	double DrainReserve(double& Reserve, double RatePerSecond, double Seconds)
	{
		const double Needed = RatePerSecond * Seconds;
		if (Needed >= Reserve)
		{
			const double EmptySeconds = Seconds - Reserve / RatePerSecond;
			Reserve = 0.0;
			return EmptySeconds;
		}
		Reserve -= Needed;
		return 0.0;
	}

	double StaminaRegenMultiplier(EAirStage Air)
	{
		switch (Air)
		{
		case EAirStage::Nominal:  return 1.0;
		case EAirStage::Impaired: return 0.5;
		default:                  return 0.0;
		}
	}
}

void FBodyModel::Advance(double Seconds, bool bDesalPowered, EAirStage Air, double DcsSeverity)
{
	if (!(Seconds > 0.0) || !FMath::IsFinite(Seconds))
	{
		return;
	}

	// Reserves run down continuously; only the time spent empty drains health.
	const double DehydratedSeconds = DrainReserve(HydrationL, WaterNeedLPerDay / SecondsPerDay, Seconds);
	const double StarvingSeconds = DrainReserve(CaloriesKcal, CalorieNeedPerDay / SecondsPerDay, Seconds);

	if (bDesalPowered)
	{
		PotableWaterL = FMath::Min(PotableTankCapacityL, PotableWaterL + DesalLPerDay / SecondsPerDay * Seconds);
	}
	Isolation = FMath::Min(1.0, Isolation + IsolationPerDay / SecondsPerDay * Seconds);

	double Drain = DehydratedSeconds * DehydratedHealthPerSecond
		+ StarvingSeconds * StarvingHealthPerSecond
		+ DcsHealthPerSecondAtFullSeverity * FMath::Clamp(DcsSeverity, 0.0, 1.0) * Seconds;
	if (Air == EAirStage::Critical)
	{
		Drain += CriticalAirHealthPerSecond * Seconds;
	}
	Health = (Air == EAirStage::Dead) ? 0.0 : FMath::Max(0.0, Health - Drain);

	Stamina = FMath::Min(100.0, Stamina + StaminaRegenPerSecond * StaminaRegenMultiplier(Air) * Seconds);
}

bool FBodyModel::EatMeal()
{
	if (MealsLeft <= 0)
	{
		return false;
	}
	--MealsLeft;
	CaloriesKcal = FMath::Min(CalorieNeedPerDay, CaloriesKcal + MealKcal);
	PotableWaterL = FMath::Max(0.0, PotableWaterL - MealWaterCostL);
	return true;
}

bool FBodyModel::Drink(double Litres)
{
	if (!(Litres > 0.0) || Litres > PotableWaterL)
	{
		return false;
	}
	PotableWaterL -= Litres;
	HydrationL = FMath::Min(WaterNeedLPerDay, HydrationL + Litres);
	return true;
}

bool FBodyModel::IsDead() const
{
	return Health <= 0.0;
}
