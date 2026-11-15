#pragma once

#include "CoreMinimal.h"

/** Decides when the opening-slice sequence must be moved to match the director. */
struct FSlicePlaybackSync
{
	/**
	 * A beat that ends on its timeout starts exactly where the sequence already is, so playback
	 * carries on untouched. A beat completed early by a gameplay event starts ahead of the
	 * sequence, which then has to jump. Unknown beats (negative start) never jump.
	 */
	static bool NeedsJump(double SequenceSeconds, double BeatStartSeconds, double ToleranceSeconds)
	{
		if (!FMath::IsFinite(SequenceSeconds) || !FMath::IsFinite(BeatStartSeconds) || BeatStartSeconds < 0.0)
		{
			return false;
		}
		return FMath::Abs(SequenceSeconds - BeatStartSeconds) > FMath::Max(ToleranceSeconds, 0.0);
	}
};
