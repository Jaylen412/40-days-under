#pragma once

#include "CoreMinimal.h"

/**
 * DualSense speaker cues: RadioStatic, DecoAlarm, RegulatorBreath, Hallucination.
 *
 * The real body needs the Sony SDK (PS5 only) and lives in a PS5-specific source file.
 * Everywhere else this is an empty inline stub so callers need no platform checks.
 */
namespace SonyPad
{
#if !PLATFORM_PS5
	inline void PlaySpeakerCue(FName Cue) {}
#else
	void PlaySpeakerCue(FName Cue);
#endif
}
