#include "Input/DualSenseFeedbackProfile.h"
#include "Simulation/AirSupplyModel.h"

namespace
{
	const FLinearColor Teal(0.0f, 0.5f, 0.5f);
	const FLinearColor Amber(1.0f, 0.6f, 0.0f);
	const FLinearColor Crimson(0.86f, 0.08f, 0.24f);

	constexpr float HordeCrimsonThreshold = 0.7f;
	constexpr double PulseHz = 1.5;
	constexpr double NitrogenWarningBandBar = 0.3;
	constexpr int32 MaxStrength = 8;
}

FLinearColor FDualSenseFeedbackProfile::LightbarColor(const FVitalsSnapshot& Snapshot, float HordeProximity01, double RealTimeSeconds)
{
	if (Snapshot.AirStage >= EAirStage::Hypoxic || HordeProximity01 >= HordeCrimsonThreshold)
	{
		const double Pulse = 0.6 + 0.4 * FMath::Sin(2.0 * UE_DOUBLE_PI * PulseHz * RealTimeSeconds);
		return FLinearColor(Crimson.R * Pulse, Crimson.G * Pulse, Crimson.B * Pulse, 1.0f);
	}

	const double FuelTerm = FMath::Clamp(1.0 - Snapshot.FuelFraction / FAirSupplyModel::SputterFraction, 0.0, 1.0);
	const double NitrogenTerm = FMath::Clamp((Snapshot.CeilingBar - Snapshot.AmbientBar + NitrogenWarningBandBar) / NitrogenWarningBandBar, 0.0, 1.0);
	const float W = static_cast<float>(FMath::Max(FuelTerm, NitrogenTerm));
	FLinearColor Out = FMath::Lerp(Teal, Amber, W);
	Out.A = 1.0f;
	return Out;
}

FTriggerFeedback FDualSenseFeedbackProfile::TriggerFor(ETriggerAction Action, float Progress01, const FVitalsSnapshot& Snapshot)
{
	FTriggerFeedback Out;
	const float Progress = FMath::Clamp(Progress01, 0.0f, 1.0f);

	switch (Action)
	{
	case ETriggerAction::SpeargunLoad:
		Out.StartPosition = 2;
		Out.EndPosition = 8;
		Out.StartStrength = 2;
		Out.EndStrength = FMath::RoundToInt(FMath::Lerp(2.0f, 8.0f, Progress));
		break;
	case ETriggerAction::SpeargunFire:
		Out.StartPosition = 6;
		Out.EndPosition = 6;
		if (Progress < 1.0f)
		{
			Out.StartStrength = MaxStrength;
			Out.EndStrength = MaxStrength;
		}
		break;
	case ETriggerAction::ValveCrank:
		Out.StartPosition = 0;
		Out.EndPosition = 8;
		Out.StartStrength = 4;
		Out.EndStrength = 4;
		Out.bVibrate = true;
		Out.VibrationFrequencyHz = 12;
		Out.VibrationAmplitude = 5;
		break;
	case ETriggerAction::None:
	default:
		break;
	}

	const int32 Fatigue = FMath::RoundToInt(3.0 * (1.0 - FMath::Clamp(Snapshot.Stamina, 0.0, 100.0) / 100.0));
	Out.StartStrength = FMath::Min(Out.StartStrength + Fatigue, MaxStrength);
	Out.EndStrength = FMath::Min(Out.EndStrength + Fatigue, MaxStrength);

	if (Snapshot.DcsSeverity > 0.0)
	{
		Out.bVibrate = true;
		Out.VibrationFrequencyHz = 8;
		Out.VibrationAmplitude = FMath::RoundToInt(8.0 * FMath::Clamp(Snapshot.DcsSeverity, 0.0, 1.0));
	}
	return Out;
}

FPumpHaptic FDualSenseFeedbackProfile::PumpHaptic(const FVitalsSnapshot& Snapshot)
{
	FPumpHaptic Out;
	Out.PulseHz = 1.6f;
	if (!Snapshot.bPumpRunning)
	{
		Out.Amplitude = 0.0f;
		Out.Jitter = 0.0f;
	}
	else if (Snapshot.bPumpSputtering)
	{
		Out.Amplitude = 0.5f;
		Out.Jitter = static_cast<float>(FMath::Clamp(1.0 - Snapshot.FuelFraction / FAirSupplyModel::SputterFraction, 0.0, 1.0));
	}
	else
	{
		Out.Amplitude = 0.35f;
		Out.Jitter = 0.0f;
	}
	return Out;
}
