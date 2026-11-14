#include "Misc/AutomationTest.h"
#include "Simulation/AirSupplyModel.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace FortyDaysUnderAirTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	constexpr double Hour = 3600.0;
	constexpr double Day = 86400.0;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAirFortyDayTankTest, "FortyDaysUnder.Air.FortyDayTank", FortyDaysUnderAirTest::Flags)
bool FAirFortyDayTankTest::RunTest(const FString& Parameters)
{
	using namespace FortyDaysUnderAirTest;

	FAirSupplyModel A;
	A.Advance(40.0 * Day);
	TestTrue(FString::Printf(TEXT("80 L left after 40 days (got %.4f)"), A.FuelL), FMath::IsNearlyEqual(A.FuelL, 80.0, 0.01));
	TestTrue(TEXT("sputtering at day 40"), A.IsSputtering());
	TestTrue(TEXT("pump running at day 40"), A.IsPumpRunning());
	TestTrue(TEXT("CO2 scrubbed at day 40"), FMath::IsNearlyEqual(A.Co2Pct, 0.04, 1e-9));
	TestEqual(TEXT("nominal at day 40"), A.Stage(), EAirStage::Nominal);

	// Day 41.667 = 1000 h from full.
	A.Advance(1000.0 * Hour - 40.0 * Day);
	TestTrue(FString::Printf(TEXT("dry at day 41.667 (got %.6f)"), A.FuelL), FMath::IsNearlyEqual(A.FuelL, 0.0, 1e-9));
	TestFalse(TEXT("pump stopped"), A.IsPumpRunning());
	TestFalse(TEXT("dead pump does not sputter"), A.IsSputtering());

	A.Advance(6.0 * Hour);
	TestTrue(FString::Printf(TEXT("+6 h CO2 ~3.04 (got %.4f)"), A.Co2Pct), FMath::IsNearlyEqual(A.Co2Pct, 3.04, 0.01));
	TestEqual(TEXT("+6 h Impaired"), A.Stage(), EAirStage::Impaired);

	A.Advance(4.0 * Hour);
	TestTrue(FString::Printf(TEXT("+10 h CO2 ~5.04 (got %.4f)"), A.Co2Pct), FMath::IsNearlyEqual(A.Co2Pct, 5.04, 0.01));
	TestEqual(TEXT("+10 h Hypoxic"), A.Stage(), EAirStage::Hypoxic);

	A.Advance(6.0 * Hour);
	TestTrue(FString::Printf(TEXT("+16 h CO2 ~8.04 (got %.4f)"), A.Co2Pct), FMath::IsNearlyEqual(A.Co2Pct, 8.04, 0.01));
	TestEqual(TEXT("+16 h Critical"), A.Stage(), EAirStage::Critical);

	A.Advance(4.0 * Hour);
	TestEqual(TEXT("+20 h Dead"), A.Stage(), EAirStage::Dead);

	// Crossing fuel-zero inside one large step gives the same result as small steps.
	FAirSupplyModel Big;
	Big.Advance(1010.0 * Hour);
	FAirSupplyModel Small;
	for (int32 i = 0; i < 1010 * 60; ++i)
	{
		Small.Advance(60.0);
	}
	TestTrue(FString::Printf(TEXT("CO2 step-size independent (%.6f vs %.6f)"), Big.Co2Pct, Small.Co2Pct), FMath::IsNearlyEqual(Big.Co2Pct, Small.Co2Pct, 1e-6));
	TestTrue(FString::Printf(TEXT("O2 step-size independent (%.6f vs %.6f)"), Big.O2Pct, Small.O2Pct), FMath::IsNearlyEqual(Big.O2Pct, Small.O2Pct, 1e-6));
	TestTrue(TEXT("CO2 after 10 h dry ~5.04"), FMath::IsNearlyEqual(Big.Co2Pct, 5.04, 1e-6));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAirFuelClampsTest, "FortyDaysUnder.Air.FuelClamps", FortyDaysUnderAirTest::Flags)
bool FAirFuelClampsTest::RunTest(const FString& Parameters)
{
	using namespace FortyDaysUnderAirTest;

	FAirSupplyModel A;
	A.FuelL = 100.0;
	TestTrue(TEXT("AddFuel(5000) on 100 L accepts 1900"), FMath::IsNearlyEqual(A.AddFuel(5000.0), 1900.0, 1e-9));
	TestTrue(TEXT("tank full at 2000"), FMath::IsNearlyEqual(A.FuelL, 2000.0, 1e-9));
	TestTrue(TEXT("AddFuel(-5) accepts 0"), FMath::IsNearlyEqual(A.AddFuel(-5.0), 0.0, 1e-12));
	TestTrue(TEXT("negative add leaves tank full"), FMath::IsNearlyEqual(A.FuelL, 2000.0, 1e-9));
	A.Advance(100.0 * Day);
	TestTrue(TEXT("fuel never below 0"), A.FuelL >= 0.0);
	TestTrue(TEXT("fuel fraction never below 0"), A.FuelFraction() >= 0.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAirRefuelRecoversTest, "FortyDaysUnder.Air.RefuelRecovers", FortyDaysUnderAirTest::Flags)
bool FAirRefuelRecoversTest::RunTest(const FString& Parameters)
{
	using namespace FortyDaysUnderAirTest;

	FAirSupplyModel A;
	A.Advance(1010.0 * Hour);
	TestEqual(TEXT("starts Hypoxic"), A.Stage(), EAirStage::Hypoxic);
	A.AddFuel(500.0);
	A.Advance(3.0 * Hour);
	TestEqual(TEXT("Nominal after 3 h"), A.Stage(), EAirStage::Nominal);
	TestTrue(FString::Printf(TEXT("CO2 back to 0.04 (got %.4f)"), A.Co2Pct), FMath::IsNearlyEqual(A.Co2Pct, 0.04, 1e-9));
	TestTrue(FString::Printf(TEXT("O2 back to 20.9 (got %.4f)"), A.O2Pct), FMath::IsNearlyEqual(A.O2Pct, 20.9, 1e-9));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
