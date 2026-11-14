#include "Misc/AutomationTest.h"
#include "UObject/Package.h"
#include "Input/DualSenseFeedbackProfile.h"
#include "Input/DualSenseFeedbackComponent.h"
#include "Simulation/VitalsSnapshot.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace FortyDaysUnderDualSenseTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FLinearColor ExpectedTeal(0.0f, 0.5f, 0.5f);
	const FLinearColor ExpectedAmber(1.0f, 0.6f, 0.0f);

	bool ColorNear(const FLinearColor& A, const FLinearColor& B, float Tol = 0.001f)
	{
		return FMath::Abs(A.R - B.R) <= Tol && FMath::Abs(A.G - B.G) <= Tol && FMath::Abs(A.B - B.B) <= Tol;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDualSenseLightbarRampTest, "FortyDaysUnder.Input.LightbarRamp", FortyDaysUnderDualSenseTest::Flags)
bool FDualSenseLightbarRampTest::RunTest(const FString& Parameters)
{
	using namespace FortyDaysUnderDualSenseTest;

	FVitalsSnapshot S;
	S.CeilingBar = 1.622;
	S.AmbientBar = 2.484;
	S.FuelFraction = 1.0;
	TestTrue(TEXT("Saturated in the lab with full fuel is teal"), ColorNear(FDualSenseFeedbackProfile::LightbarColor(S, 0.f, 0.0), ExpectedTeal));

	S.FuelFraction = 0.075;
	TestTrue(TEXT("Fuel 0.075 is halfway teal/amber"), ColorNear(FDualSenseFeedbackProfile::LightbarColor(S, 0.f, 0.0), FMath::Lerp(ExpectedTeal, ExpectedAmber, 0.5f)));

	S.FuelFraction = 0.0;
	TestTrue(TEXT("Fuel 0 is amber"), ColorNear(FDualSenseFeedbackProfile::LightbarColor(S, 0.f, 0.0), ExpectedAmber));

	FVitalsSnapshot N;
	N.CeilingBar = 1.35;
	N.AmbientBar = 1.5;
	N.FuelFraction = 1.0;
	TestTrue(TEXT("Ceiling 0.15 bar under ambient is halfway teal/amber"), ColorNear(FDualSenseFeedbackProfile::LightbarColor(N, 0.f, 0.0), FMath::Lerp(ExpectedTeal, ExpectedAmber, 0.5f)));

	FVitalsSnapshot H;
	H.AirStage = EAirStage::Hypoxic;
	const FLinearColor C0 = FDualSenseFeedbackProfile::LightbarColor(H, 0.f, 0.0);
	const FLinearColor C1 = FDualSenseFeedbackProfile::LightbarColor(H, 0.f, 1.0 / 6.0);
	TestTrue(TEXT("Hypoxic red channel dominates"), C0.R > C0.G && C0.R > C0.B);
	TestTrue(TEXT("Hypoxic pulses over time"), !FMath::IsNearlyEqual(C0.R, C1.R, 0.01f));
	TestEqual(TEXT("Pulse at t=0 is 0.6x crimson"), C0.R, 0.86f * 0.6f, 0.001f);
	TestEqual(TEXT("Pulse peak is 1.0x crimson"), C1.R, 0.86f, 0.001f);

	FVitalsSnapshot Saturated;
	Saturated.CeilingBar = 1.622;
	Saturated.AmbientBar = 2.484;
	const FLinearColor Horde = FDualSenseFeedbackProfile::LightbarColor(Saturated, 0.8f, 1.0 / 6.0);
	TestTrue(TEXT("Horde proximity 0.8 is crimson family"), Horde.R > Horde.G && Horde.R > Horde.B);
	const FLinearColor NotYet = FDualSenseFeedbackProfile::LightbarColor(Saturated, 0.69f, 1.0 / 6.0);
	TestTrue(TEXT("Horde proximity 0.69 is not crimson"), ColorNear(NotYet, ExpectedTeal));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDualSenseTriggerCurvesTest, "FortyDaysUnder.Input.TriggerCurves", FortyDaysUnderDualSenseTest::Flags)
bool FDualSenseTriggerCurvesTest::RunTest(const FString& Parameters)
{
	FVitalsSnapshot S;

	const FTriggerFeedback Load0 = FDualSenseFeedbackProfile::TriggerFor(ETriggerAction::SpeargunLoad, 0.f, S);
	TestEqual(TEXT("Load start position"), Load0.StartPosition, 2);
	TestEqual(TEXT("Load end position"), Load0.EndPosition, 8);
	TestEqual(TEXT("Load progress 0 start strength"), Load0.StartStrength, 2);
	TestEqual(TEXT("Load progress 0 end strength"), Load0.EndStrength, 2);
	TestFalse(TEXT("Load does not vibrate"), Load0.bVibrate);

	const FTriggerFeedback Load1 = FDualSenseFeedbackProfile::TriggerFor(ETriggerAction::SpeargunLoad, 1.f, S);
	TestEqual(TEXT("Load progress 1 end strength"), Load1.EndStrength, 8);

	S.Stamina = 0.0;
	const FTriggerFeedback Tired0 = FDualSenseFeedbackProfile::TriggerFor(ETriggerAction::SpeargunLoad, 0.f, S);
	TestEqual(TEXT("Zero stamina adds 3 to start"), Tired0.StartStrength, 5);
	TestEqual(TEXT("Zero stamina adds 3 to end"), Tired0.EndStrength, 5);
	const FTriggerFeedback Tired1 = FDualSenseFeedbackProfile::TriggerFor(ETriggerAction::SpeargunLoad, 1.f, S);
	TestEqual(TEXT("Fatigue is capped at 8"), Tired1.EndStrength, 8);

	FVitalsSnapshot D;
	D.DcsSeverity = 0.5;
	const FTriggerFeedback Dcs = FDualSenseFeedbackProfile::TriggerFor(ETriggerAction::SpeargunLoad, 0.f, D);
	TestTrue(TEXT("DCS vibrates"), Dcs.bVibrate);
	TestEqual(TEXT("DCS 8 Hz"), Dcs.VibrationFrequencyHz, 8);
	TestEqual(TEXT("DCS amplitude 4"), Dcs.VibrationAmplitude, 4);

	const FTriggerFeedback None = FDualSenseFeedbackProfile::TriggerFor(ETriggerAction::None, 0.f, FVitalsSnapshot());
	TestEqual(TEXT("None start strength"), None.StartStrength, 0);
	TestEqual(TEXT("None end strength"), None.EndStrength, 0);
	TestFalse(TEXT("None does not vibrate"), None.bVibrate);

	const FTriggerFeedback Fire = FDualSenseFeedbackProfile::TriggerFor(ETriggerAction::SpeargunFire, 0.5f, FVitalsSnapshot());
	TestEqual(TEXT("Fire wall position"), Fire.StartPosition, 6);
	TestEqual(TEXT("Fire wall strength"), Fire.StartStrength, 8);
	const FTriggerFeedback Released = FDualSenseFeedbackProfile::TriggerFor(ETriggerAction::SpeargunFire, 1.f, FVitalsSnapshot());
	TestEqual(TEXT("Fire released"), Released.StartStrength + Released.EndStrength, 0);

	const FTriggerFeedback Valve = FDualSenseFeedbackProfile::TriggerFor(ETriggerAction::ValveCrank, 0.f, FVitalsSnapshot());
	TestTrue(TEXT("Valve vibrates"), Valve.bVibrate);
	TestEqual(TEXT("Valve 12 Hz"), Valve.VibrationFrequencyHz, 12);
	TestEqual(TEXT("Valve amplitude 5"), Valve.VibrationAmplitude, 5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDualSensePumpHapticTest, "FortyDaysUnder.Input.PumpHaptic", FortyDaysUnderDualSenseTest::Flags)
bool FDualSensePumpHapticTest::RunTest(const FString& Parameters)
{
	FVitalsSnapshot S;
	S.FuelFraction = 1.0;
	const FPumpHaptic Steady = FDualSenseFeedbackProfile::PumpHaptic(S);
	TestEqual(TEXT("Steady Hz"), Steady.PulseHz, 1.6f, 0.0001f);
	TestEqual(TEXT("Steady amplitude"), Steady.Amplitude, 0.35f, 0.0001f);
	TestEqual(TEXT("Steady jitter"), Steady.Jitter, 0.0f, 0.0001f);

	S.FuelFraction = 0.075;
	S.bPumpSputtering = true;
	const FPumpHaptic Sputter = FDualSenseFeedbackProfile::PumpHaptic(S);
	TestEqual(TEXT("Sputter jitter"), Sputter.Jitter, 0.5f, 0.0001f);
	TestEqual(TEXT("Sputter amplitude"), Sputter.Amplitude, 0.5f, 0.0001f);

	S.bPumpSputtering = false;
	S.bPumpRunning = false;
	TestEqual(TEXT("Dead pump amplitude"), FDualSenseFeedbackProfile::PumpHaptic(S).Amplitude, 0.0f, 0.0001f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDualSenseNoDeviceTest, "FortyDaysUnder.Input.NoDeviceIsNoOp", FortyDaysUnderDualSenseTest::Flags)
bool FDualSenseNoDeviceTest::RunTest(const FString& Parameters)
{
	UDualSenseFeedbackComponent* Component = NewObject<UDualSenseFeedbackComponent>(GetTransientPackage());
	TestNotNull(TEXT("Component created"), Component);
	TestFalse(TEXT("No supported device"), Component->HasSupportedDevice());
	Component->SetTriggerAction(ETriggerAction::SpeargunLoad, 0.5f);
	Component->SetHordeProximity(0.9f);
	FVitalsSnapshot S;
	S.DcsSeverity = 0.5;
	Component->HandleVitalsChanged(S);
	TestFalse(TEXT("Still no supported device"), Component->HasSupportedDevice());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDualSenseOnePropertyPerTriggerTest, "FortyDaysUnder.Input.OnePropertyPerTrigger", FortyDaysUnderDualSenseTest::Flags)
bool FDualSenseOnePropertyPerTriggerTest::RunTest(const FString& Parameters)
{
	FTriggerFeedback F;
	TestTrue(TEXT("All zero resets"), FDualSenseFeedbackProfile::SendKindFor(F) == ETriggerSendKind::Reset);
	F.EndStrength = 2;
	TestTrue(TEXT("Strength without vibration is resistance"), FDualSenseFeedbackProfile::SendKindFor(F) == ETriggerSendKind::Resistance);
	F.bVibrate = true;
	TestTrue(TEXT("Vibration wins"), FDualSenseFeedbackProfile::SendKindFor(F) == ETriggerSendKind::Vibration);
	F.EndStrength = 0;
	TestTrue(TEXT("Vibration without strength is vibration"), FDualSenseFeedbackProfile::SendKindFor(F) == ETriggerSendKind::Vibration);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDualSenseDeviceUnitScalingTest, "FortyDaysUnder.Input.DeviceUnitScaling", FortyDaysUnderDualSenseTest::Flags)
bool FDualSenseDeviceUnitScalingTest::RunTest(const FString& Parameters)
{
#if defined(PLATFORM_PS5) && PLATFORM_PS5
	return true; // pass-through on PS5; verify on hardware
#else
	FTriggerFeedback Max;
	Max.StartPosition = Max.EndPosition = Max.StartStrength = Max.EndStrength = Max.VibrationAmplitude = 8;
	Max.VibrationFrequencyHz = 12;
	const FTriggerFeedback M = FDualSenseFeedbackProfile::ToDeviceUnits(Max);
	TestEqual(TEXT("Position 8 -> 255"), M.StartPosition, 255);
	TestEqual(TEXT("End position 8 -> 255"), M.EndPosition, 255);
	TestEqual(TEXT("Strength 8 -> 255"), M.StartStrength, 255);
	TestEqual(TEXT("End strength 8 -> 255"), M.EndStrength, 255);
	TestEqual(TEXT("Amplitude 8 -> 255"), M.VibrationAmplitude, 255);
	TestEqual(TEXT("Frequency 12 Hz -> 255"), M.VibrationFrequencyHz, 255);

	const FTriggerFeedback Z = FDualSenseFeedbackProfile::ToDeviceUnits(FTriggerFeedback());
	TestEqual(TEXT("Strength 0 -> 0"), Z.StartStrength, 0);
	TestEqual(TEXT("Position 0 -> 0"), Z.StartPosition, 0);
	TestEqual(TEXT("Amplitude 0 -> 0"), Z.VibrationAmplitude, 0);

	int32 Previous = -1;
	for (int32 V = 0; V <= 8; ++V)
	{
		FTriggerFeedback D;
		D.StartStrength = V;
		const int32 Scaled = FDualSenseFeedbackProfile::ToDeviceUnits(D).StartStrength;
		TestTrue(*FString::Printf(TEXT("Monotonic at %d"), V), Scaled > Previous);
		Previous = Scaled;
	}
	return true;
#endif
}

#endif
