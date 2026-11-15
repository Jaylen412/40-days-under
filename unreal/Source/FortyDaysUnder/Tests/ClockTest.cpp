#include "Misc/AutomationTest.h"
#include "UObject/StrongObjectPtr.h"
#include "Engine/GameInstance.h"
#include "Survival/SurvivalClockSubsystem.h"
#include "Survival/SurvivalVitalsComponent.h"
#include "Simulation/BuhlmannModel.h"
#include "Tests/VitalsTestListener.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace FortyDaysUnderClockTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	struct FNamedDouble
	{
		const TCHAR* Name;
		double Value;
	};

	TArray<FNamedDouble> Doubles(const FVitalsSnapshot& S)
	{
		return {
			{ TEXT("GameSeconds"), S.GameSeconds },
			{ TEXT("FuelFraction"), S.FuelFraction },
			{ TEXT("Co2Pct"), S.Co2Pct },
			{ TEXT("O2Pct"), S.O2Pct },
			{ TEXT("AmbientBar"), S.AmbientBar },
			{ TEXT("CeilingBar"), S.CeilingBar },
			{ TEXT("DcsSeverity"), S.DcsSeverity },
			{ TEXT("DecoHoursRemaining"), S.DecoHoursRemaining },
			{ TEXT("Health"), S.Health },
			{ TEXT("Stamina"), S.Stamina },
			{ TEXT("HydrationL"), S.HydrationL },
			{ TEXT("CaloriesKcal"), S.CaloriesKcal },
			{ TEXT("Isolation"), S.Isolation },
		};
	}

	/** Compares every field of two snapshots; doubles within Tolerance. */
	void ExpectSnapshotsEqual(FAutomationTestBase& Test, const FVitalsSnapshot& A, const FVitalsSnapshot& B, double Tolerance)
	{
		const TArray<FNamedDouble> DA = Doubles(A);
		const TArray<FNamedDouble> DB = Doubles(B);
		for (int32 i = 0; i < DA.Num(); ++i)
		{
			Test.TestTrue(FString::Printf(TEXT("%s equal (%.9f vs %.9f)"), DA[i].Name, DA[i].Value, DB[i].Value), FMath::IsNearlyEqual(DA[i].Value, DB[i].Value, Tolerance));
		}
		Test.TestEqual(TEXT("Day equal"), A.Day, B.Day);
		Test.TestEqual(TEXT("bPumpRunning equal"), A.bPumpRunning, B.bPumpRunning);
		Test.TestEqual(TEXT("bPumpSputtering equal"), A.bPumpSputtering, B.bPumpSputtering);
		Test.TestEqual(TEXT("AirStage equal"), A.AirStage, B.AirStage);
		Test.TestEqual(TEXT("bInDecoCycle equal"), A.bInDecoCycle, B.bInDecoCycle);
		Test.TestEqual(TEXT("bDead equal"), A.bDead, B.bDead);
	}

	void ExpectNoNaN(FAutomationTestBase& Test, const FVitalsSnapshot& S)
	{
		for (const FNamedDouble& D : Doubles(S))
		{
			Test.TestTrue(FString::Printf(TEXT("%s is finite"), D.Name), FMath::IsFinite(D.Value));
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClockLargeJumpMatchesStepsTest, "FortyDaysUnder.Clock.LargeJumpMatchesSteps", FortyDaysUnderClockTest::Flags)
bool FClockLargeJumpMatchesStepsTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<USurvivalVitalsComponent> A(NewObject<USurvivalVitalsComponent>());
	TStrongObjectPtr<USurvivalVitalsComponent> B(NewObject<USurvivalVitalsComponent>());
	for (int32 i = 0; i < 1440; ++i)
	{
		A->SimStep(60.0);
	}
	B->SimStep(86400.0);

	TestTrue(TEXT("a full day elapsed"), FMath::IsNearlyEqual(A->GetSnapshot().GameSeconds, 86400.0, 1e-6));
	TestEqual(TEXT("day 2"), A->GetSnapshot().Day, 2);
	FortyDaysUnderClockTest::ExpectSnapshotsEqual(*this, A->GetSnapshot(), B->GetSnapshot(), 1e-6);

	// A non-multiple of the step size at constant depth: 3 × 100 s vs one 300 s call.
	TStrongObjectPtr<USurvivalVitalsComponent> C(NewObject<USurvivalVitalsComponent>());
	TStrongObjectPtr<USurvivalVitalsComponent> D(NewObject<USurvivalVitalsComponent>());
	C->SimStep(100.0);
	C->SimStep(100.0);
	C->SimStep(100.0);
	D->SimStep(300.0);
	FortyDaysUnderClockTest::ExpectSnapshotsEqual(*this, C->GetSnapshot(), D->GetSnapshot(), 1e-6);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClockZeroAndNegativeDeltaTest, "FortyDaysUnder.Clock.ZeroAndNegativeDelta", FortyDaysUnderClockTest::Flags)
bool FClockZeroAndNegativeDeltaTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<USurvivalVitalsComponent> V(NewObject<USurvivalVitalsComponent>());
	const FVitalsSnapshot Before = V->GetSnapshot();
	V->SimStep(0.0);
	V->SimStep(-10.0);
	FortyDaysUnderClockTest::ExpectSnapshotsEqual(*this, Before, V->GetSnapshot(), 0.0);
	FortyDaysUnderClockTest::ExpectNoNaN(*this, V->GetSnapshot());

	TStrongObjectPtr<UGameInstance> GameInstance(NewObject<UGameInstance>());
	TStrongObjectPtr<USurvivalClockSubsystem> Clock(NewObject<USurvivalClockSubsystem>(GameInstance.Get()));
	int32 Fired = 0;
	Clock->OnSimStep.AddLambda([&Fired](double) { ++Fired; });
	Clock->TimeScale = 0.0;
	Clock->Tick(1.0f);
	TestEqual(TEXT("paused clock fires no steps"), Fired, 0);
	TestEqual(TEXT("paused clock does not advance"), Clock->GetGameSeconds(), 0.0);

	Clock->AdvanceBy(-10.0);
	Clock->AdvanceBy(0.0);
	TestEqual(TEXT("non-positive AdvanceBy fires no steps"), Fired, 0);

	Clock->TimeScale = 60.0;
	Clock->Tick(1.0f);
	TestEqual(TEXT("1 real second at 60x fires one step"), Fired, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClockFixedStepsTest, "FortyDaysUnder.Clock.FixedSteps", FortyDaysUnderClockTest::Flags)
bool FClockFixedStepsTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UGameInstance> GameInstance(NewObject<UGameInstance>());
	TStrongObjectPtr<USurvivalClockSubsystem> Clock(NewObject<USurvivalClockSubsystem>(GameInstance.Get()));
	TArray<double> Steps;
	Clock->OnSimStep.AddLambda([&Steps](double StepSeconds) { Steps.Add(StepSeconds); });

	Clock->AdvanceBy(150.0);
	TestEqual(TEXT("150 s fires two steps"), Steps.Num(), 2);
	for (double S : Steps)
	{
		TestEqual(TEXT("each step is 60 s"), S, 60.0);
	}

	Clock->AdvanceBy(30.0);
	TestEqual(TEXT("remainder carried: third step fires"), Steps.Num(), 3);
	TestEqual(TEXT("third step is 60 s"), Steps.Last(), 60.0);
	TestEqual(TEXT("game seconds 180"), Clock->GetGameSeconds(), 180.0);
	TestEqual(TEXT("still day 1"), Clock->GetDay(), 1);

	Clock->AdvanceBy(86400.0);
	TestEqual(TEXT("day 2"), Clock->GetDay(), 2);
	TestEqual(TEXT("1440 more steps"), Steps.Num(), 3 + 1440);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVitalsDecoCycleThroughComponentTest, "FortyDaysUnder.Vitals.DecoCycleThroughComponent", FortyDaysUnderClockTest::Flags)
bool FVitalsDecoCycleThroughComponentTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<USurvivalVitalsComponent> V(NewObject<USurvivalVitalsComponent>());
	TestTrue(TEXT("starts at 15 m"), FMath::IsNearlyEqual(V->GetSnapshot().AmbientBar, FBuhlmannModel::AmbientBarAtDepth(15.0), 1e-9));

	V->BeginDecoCycle();
	TestTrue(TEXT("in deco cycle"), V->GetSnapshot().bInDecoCycle);
	TestTrue(TEXT("17 h remaining"), FMath::IsNearlyEqual(V->GetSnapshot().DecoHoursRemaining, 17.0, 1e-9));

	double MaxSeverity = 0.0;
	for (int32 Minute = 0; Minute < 17 * 60; ++Minute)
	{
		V->SimStep(60.0);
		MaxSeverity = FMath::Max(MaxSeverity, V->GetSnapshot().DcsSeverity);
	}
	const FVitalsSnapshot& S = V->GetSnapshot();
	TestFalse(TEXT("cycle finished"), S.bInDecoCycle);
	TestEqual(TEXT("no DCS at any step"), MaxSeverity, 0.0);
	TestEqual(TEXT("no hours remaining"), S.DecoHoursRemaining, 0.0);
	TestTrue(FString::Printf(TEXT("at surface pressure (got %.6f)"), S.AmbientBar), FMath::IsNearlyEqual(S.AmbientBar, 1.01325, 1e-9));
	TestTrue(TEXT("ceiling at or below surface"), S.CeilingBar <= FBuhlmannModel::SurfacePressureBar);

	// Ascending early: abort at 8 h and surface.
	TStrongObjectPtr<USurvivalVitalsComponent> W(NewObject<USurvivalVitalsComponent>());
	W->BeginDecoCycle();
	for (int32 Minute = 0; Minute < 8 * 60; ++Minute)
	{
		W->SimStep(60.0);
	}
	W->AbortDecoCycle();
	TestFalse(TEXT("aborted"), W->GetSnapshot().bInDecoCycle);
	W->SetDepthMetres(0.0);
	const double HealthBefore = W->GetSnapshot().Health;
	W->SimStep(60.0);
	TestTrue(FString::Printf(TEXT("DCS after early surfacing (got %.4f)"), W->GetSnapshot().DcsSeverity), W->GetSnapshot().DcsSeverity > 0.0);
	TestTrue(TEXT("health falling"), W->GetSnapshot().Health < HealthBefore);
	const double HealthMid = W->GetSnapshot().Health;
	W->SimStep(60.0);
	TestTrue(TEXT("health keeps falling"), W->GetSnapshot().Health < HealthMid);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVitalsDeathFiresOnceTest, "FortyDaysUnder.Vitals.DeathFiresOnce", FortyDaysUnderClockTest::Flags)
bool FVitalsDeathFiresOnceTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<USurvivalVitalsComponent> V(NewObject<USurvivalVitalsComponent>());
	TStrongObjectPtr<UVitalsTestListener> L(NewObject<UVitalsTestListener>());
	V->OnDied.AddDynamic(L.Get(), &UVitalsTestListener::HandleDied);
	V->OnVitalsChanged.AddDynamic(L.Get(), &UVitalsTestListener::HandleVitalsChanged);

	// Surfacing from saturation without deco: full DCS, 0.5 health/s.
	V->SetDepthMetres(0.0);
	V->SimStep(600.0);
	TestTrue(TEXT("dead"), V->GetSnapshot().bDead);
	TestEqual(TEXT("health 0"), V->GetSnapshot().Health, 0.0);
	TestEqual(TEXT("OnDied fired once"), L->DiedCount, 1);
	TestTrue(TEXT("OnVitalsChanged fired"), L->ChangedCount > 0);

	for (int32 i = 0; i < 100; ++i)
	{
		V->SimStep(60.0);
	}
	V->SimStep(86400.0);
	TestEqual(TEXT("OnDied still fired once"), L->DiedCount, 1);
	TestTrue(TEXT("bDead stays true"), V->GetSnapshot().bDead);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
