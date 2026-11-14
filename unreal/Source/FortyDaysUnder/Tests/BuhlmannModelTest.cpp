#include "Misc/AutomationTest.h"
#include "Simulation/BuhlmannModel.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace FortyDaysUnderDecoTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDecoSaturatedDiverCannotSurfaceTest, "FortyDaysUnder.Deco.SaturatedDiverCannotSurface", FortyDaysUnderDecoTest::Flags)
bool FDecoSaturatedDiverCannotSurfaceTest::RunTest(const FString& Parameters)
{
	FBuhlmannModel M;
	M.ResetToSaturationAt(15.0, 0.79);
	TestTrue(FString::Printf(TEXT("ceiling ~1.622 bar (got %.5f)"), M.CeilingBar()), FMath::IsNearlyEqual(M.CeilingBar(), 1.622, 0.002));
	TestFalse(TEXT("unsafe at surface"), M.IsSafeAt(FBuhlmannModel::SurfacePressureBar));
	TestTrue(TEXT("safe at depth"), M.IsSafeAt(FBuhlmannModel::AmbientBarAtDepth(15.0)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDecoSeventeenHourCycleClearsTest, "FortyDaysUnder.Deco.SeventeenHourCycleClears", FortyDaysUnderDecoTest::Flags)
bool FDecoSeventeenHourCycleClearsTest::RunTest(const FString& Parameters)
{
	FBuhlmannModel M;
	M.ResetToSaturationAt(15.0, 0.79);

	int32 FirstUnsafeMinute = INDEX_NONE;
	const int32 TotalMinutes = 17 * 60;
	for (int32 Minute = 1; Minute <= TotalMinutes; ++Minute)
	{
		const double Ambient = FBuhlmannModel::DecoAmbientBar(Minute / 60.0);
		M.Advance(60.0, Ambient, FBuhlmannModel::DecoGasFN2);
		if (FirstUnsafeMinute == INDEX_NONE && !M.IsSafeAt(Ambient))
		{
			FirstUnsafeMinute = Minute;
		}
		if (Minute == 16 * 60)
		{
			TestTrue(FString::Printf(TEXT("after 16 h ceiling above surface (got %.5f)"), M.CeilingBar()), M.CeilingBar() > FBuhlmannModel::SurfacePressureBar);
			TestTrue(TEXT("after 16 h ceiling ~1.028"), FMath::IsNearlyEqual(M.CeilingBar(), 1.028, 0.002));
		}
	}
	TestEqual(TEXT("safe at every step of the schedule"), FirstUnsafeMinute, (int32)INDEX_NONE);
	TestTrue(FString::Printf(TEXT("after 17 h ceiling at or below surface (got %.5f)"), M.CeilingBar()), M.CeilingBar() <= FBuhlmannModel::SurfacePressureBar);
	TestTrue(TEXT("after 17 h ceiling ~0.985"), FMath::IsNearlyEqual(M.CeilingBar(), 0.985, 0.002));
	TestTrue(TEXT("schedule ends at surface"), FMath::IsNearlyEqual(FBuhlmannModel::DecoAmbientBar(17.0), FBuhlmannModel::SurfacePressureBar, 1e-9));
	TestTrue(TEXT("schedule clamps past the end"), FMath::IsNearlyEqual(FBuhlmannModel::DecoAmbientBar(30.0), FBuhlmannModel::SurfacePressureBar, 1e-9));
	TestTrue(TEXT("schedule starts at 15 m"), FMath::IsNearlyEqual(FBuhlmannModel::DecoAmbientBar(0.0), FBuhlmannModel::AmbientBarAtDepth(15.0), 1e-9));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDecoOnGassingFromSurfaceTest, "FortyDaysUnder.Deco.OnGassingFromSurface", FortyDaysUnderDecoTest::Flags)
bool FDecoOnGassingFromSurfaceTest::RunTest(const FString& Parameters)
{
	const double LabAmbient = FBuhlmannModel::AmbientBarAtDepth(FBuhlmannModel::LabDepthM);

	FBuhlmannModel TwoHours;
	TwoHours.ResetToSurfaceEquilibrium();
	TwoHours.Advance(2.0 * 3600.0, LabAmbient, FBuhlmannModel::AirFN2);
	TestTrue(FString::Printf(TEXT("2 h ceiling ~1.076 (got %.5f)"), TwoHours.CeilingBar()), FMath::IsNearlyEqual(TwoHours.CeilingBar(), 1.076, 0.005));
	TestFalse(TEXT("2 h: unsafe to surface"), TwoHours.IsSafeAt(FBuhlmannModel::SurfacePressureBar));

	FBuhlmannModel OneHour;
	OneHour.ResetToSurfaceEquilibrium();
	OneHour.Advance(3600.0, LabAmbient, FBuhlmannModel::AirFN2);
	TestTrue(FString::Printf(TEXT("1 h ceiling ~0.857 (got %.5f)"), OneHour.CeilingBar()), FMath::IsNearlyEqual(OneHour.CeilingBar(), 0.857, 0.005));
	TestTrue(TEXT("1 h: safe to surface"), OneHour.IsSafeAt(FBuhlmannModel::SurfacePressureBar));

	// Exact exponential: one 2 h step equals 120 one-minute steps.
	FBuhlmannModel Stepped;
	Stepped.ResetToSurfaceEquilibrium();
	for (int32 i = 0; i < 120; ++i)
	{
		Stepped.Advance(60.0, LabAmbient, FBuhlmannModel::AirFN2);
	}
	TestTrue(TEXT("step-size independent"), FMath::IsNearlyEqual(Stepped.CeilingBar(), TwoHours.CeilingBar(), 1e-9));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDecoSeverityRangeTest, "FortyDaysUnder.Deco.SeverityRange", FortyDaysUnderDecoTest::Flags)
bool FDecoSeverityRangeTest::RunTest(const FString& Parameters)
{
	FBuhlmannModel M;
	M.ResetToSaturationAt(15.0, 0.79);
	const double Expected = FMath::Clamp((1.622 - 1.01325) / 0.6, 0.0, 1.0);
	TestTrue(FString::Printf(TEXT("surface severity clamps to 1 (got %.5f)"), M.DcsSeverityAt(1.01325)), FMath::IsNearlyEqual(M.DcsSeverityAt(1.01325), Expected, 1e-6));
	TestEqual(TEXT("surface severity is 1.0"), M.DcsSeverityAt(1.01325), 1.0);
	TestEqual(TEXT("no severity at 2.48 bar"), M.DcsSeverityAt(2.48), 0.0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
