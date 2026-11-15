#include "Misc/AutomationTest.h"
#include "Simulation/BodyModel.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace FortyDaysUnderBodyTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	constexpr double Day = 86400.0;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBodyNeedsTest, "FortyDaysUnder.Body.Needs", FortyDaysUnderBodyTest::Flags)
bool FBodyNeedsTest::RunTest(const FString& Parameters)
{
	using namespace FortyDaysUnderBodyTest;

	FBodyModel B;
	B.Advance(Day, true, EAirStage::Nominal, 0.0);
	TestTrue(FString::Printf(TEXT("hydration spent (got %.6f)"), B.HydrationL), FMath::IsNearlyEqual(B.HydrationL, 0.0, 1e-6));
	TestTrue(FString::Printf(TEXT("calories spent (got %.6f)"), B.CaloriesKcal), FMath::IsNearlyEqual(B.CaloriesKcal, 0.0, 1e-6));
	TestTrue(FString::Printf(TEXT("desal filled tank to 40 (got %.6f)"), B.PotableWaterL), FMath::IsNearlyEqual(B.PotableWaterL, 40.0, 1e-6));
	TestTrue(FString::Printf(TEXT("isolation 0.025 (got %.6f)"), B.Isolation), FMath::IsNearlyEqual(B.Isolation, 0.025, 1e-9));
	TestFalse(TEXT("alive after day 1"), B.IsDead());

	TestTrue(TEXT("EatMeal succeeds"), B.EatMeal());
	TestEqual(TEXT("699 meals left"), B.MealsLeft, 699);
	TestTrue(TEXT("+800 kcal"), FMath::IsNearlyEqual(B.CaloriesKcal, 800.0, 1e-6));
	TestTrue(TEXT("meal costs 0.25 L of potable water"), FMath::IsNearlyEqual(B.PotableWaterL, 39.75, 1e-6));

	FBodyModel Capped;
	TestTrue(TEXT("EatMeal on full stomach succeeds"), Capped.EatMeal());
	TestTrue(TEXT("calories capped at 2400"), FMath::IsNearlyEqual(Capped.CaloriesKcal, 2400.0, 1e-9));

	FBodyModel NoFood;
	NoFood.MealsLeft = 0;
	TestFalse(TEXT("EatMeal fails with no meals"), NoFood.EatMeal());

	FBodyModel Thirsty;
	Thirsty.HydrationL = 1.0;
	TestTrue(TEXT("Drink 1 L succeeds"), Thirsty.Drink(1.0));
	TestTrue(TEXT("drink moves water to body"), FMath::IsNearlyEqual(Thirsty.HydrationL, 2.0, 1e-9));
	TestTrue(TEXT("drink draws on the tank"), FMath::IsNearlyEqual(Thirsty.PotableWaterL, 19.0, 1e-9));
	TestFalse(TEXT("Drink more than the tank holds fails"), Thirsty.Drink(500.0));

	B.Advance(3.0 * Day, true, EAirStage::Nominal, 0.0);
	TestTrue(FString::Printf(TEXT("dead after 3 days dehydrated (health %.4f)"), B.Health), B.IsDead());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBodyDcsDrainTest, "FortyDaysUnder.Body.DcsDrain", FortyDaysUnderBodyTest::Flags)
bool FBodyDcsDrainTest::RunTest(const FString& Parameters)
{
	FBodyModel B;
	B.Advance(200.0, true, EAirStage::Nominal, 1.0);
	TestTrue(FString::Printf(TEXT("health 0 after 200 s of full DCS (got %.6f)"), B.Health), FMath::IsNearlyEqual(B.Health, 0.0, 1e-9));
	TestTrue(TEXT("dead"), B.IsDead());

	FBodyModel Half;
	Half.Advance(100.0, true, EAirStage::Nominal, 0.5);
	TestTrue(TEXT("half severity drains 25 in 100 s"), FMath::IsNearlyEqual(Half.Health, 75.0, 1e-9));

	FBodyModel Suffocated;
	Suffocated.Advance(1.0, true, EAirStage::Dead, 0.0);
	TestTrue(TEXT("Dead air sets health to 0"), Suffocated.IsDead());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
