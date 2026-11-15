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

/** Which single trigger property to send. A trigger holds one mode, so only one is ever sent. */
enum class ETriggerSendKind : uint8
{
	Reset,       // all strengths 0 and no vibration: trigger off
	Resistance,  // non-zero strength, no vibration
	Vibration    // vibration wins; a later property replaces an earlier one on the same trigger
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

	/** Decides the one property to send for a trigger. */
	static ETriggerSendKind SendKindFor(const FTriggerFeedback& Feedback);

	/**
	 * Converts the 0..8 design scale into what the platform input interface expects.
	 * This is the only place the device range lives. Under PLATFORM_PS5 values pass through
	 * unchanged. Elsewhere (the engine's Apple DualSense path) position, strength and amplitude
	 * are divided by 255 there, so 8 maps to 255; frequency is normalised against the 12 Hz maximum.
	 * Both scales must be verified on hardware.
	 */
	static FTriggerFeedback ToDeviceUnits(const FTriggerFeedback& Design);
};
