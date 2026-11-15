#pragma once

#include "CoreMinimal.h"
#include "Simulation/VitalsSnapshot.h"

/** Christian's body and the station's food and water stores. Plain struct, no UObject. */
struct FBodyModel
{
	static constexpr double WaterNeedLPerDay = 3.0;
	static constexpr double CalorieNeedPerDay = 2400.0;
	static constexpr double MealKcal = 800.0;
	static constexpr double MealWaterCostL = 0.25;
	static constexpr int32  StartingMeals = 700;
	static constexpr double DesalLPerDay = 20.0;
	static constexpr double IsolationPerDay = 0.025;   // reaches 1.0 on day 40
	static constexpr double StaminaRegenPerSecond = 5.0; // at Nominal air; not set by the brief
	static constexpr double PotableTankCapacityL = 200.0;

	double Health = 100.0;         // 0..100
	double Stamina = 100.0;        // 0..100
	double HydrationL = 3.0;       // body reserve, 0..3
	double CaloriesKcal = 2400.0;  // body reserve, 0..2400
	double Isolation = 0.0;        // 0..1
	double PotableWaterL = 20.0;   // station tank, 0..200
	int32  MealsLeft = StartingMeals;

	void Advance(double Seconds, bool bDesalPowered, EAirStage Air, double DcsSeverity);
	bool EatMeal();     // false if none left
	bool Drink(double Litres); // false if tank lacks it
	bool IsDead() const;
};
