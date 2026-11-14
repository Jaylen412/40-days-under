#pragma once

#include "CoreMinimal.h"
#include "Simulation/VitalsSnapshot.h"
#include "DualSenseFeedbackProfile.generated.h"

/** What the player's right trigger is currently doing. */
UENUM(BlueprintType)
enum class ETriggerAction : uint8
{
	None,
	SpeargunLoad,
	SpeargunFire,
	ValveCrank
};

/** Adaptive trigger settings; positions and strengths are 0..8. */
struct FTriggerFeedback
{
	int32 StartPosition = 0;
	int32 EndPosition = 0;
	int32 StartStrength = 0;
	int32 EndStrength = 0;
	bool bVibrate = false;
	int32 VibrationFrequencyHz = 0;
	int32 VibrationAmplitude = 0;
};

/** Pump heartbeat haptic. Jitter 0 means steady. */
struct FPumpHaptic
{
	float PulseHz = 0.0f;
	float Amplitude = 0.0f;
	float Jitter = 0.0f;
};

/** Pure mapping from survival state to DualSense output. No engine state. */
struct FORTYDAYSUNDER_API FDualSenseFeedbackProfile
{
	static FLinearColor LightbarColor(const FVitalsSnapshot& Snapshot, float HordeProximity01, double RealTimeSeconds);
	static FTriggerFeedback TriggerFor(ETriggerAction Action, float Progress01, const FVitalsSnapshot& Snapshot);
	static FPumpHaptic PumpHaptic(const FVitalsSnapshot& Snapshot);
};
