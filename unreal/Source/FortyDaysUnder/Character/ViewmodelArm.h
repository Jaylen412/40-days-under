#pragma once

#include "CoreMinimal.h"

/**
 * One first-person forearm that eases between a rest pose (below the view) and a raised pose while its input
 * is held. Plain struct so the motion can be tested without a world.
 */
struct FViewmodelArm
{
	/** 0 = rest, 1 = raised (linear progress; Eased() shapes it). */
	double Alpha = 0.0;
	double RaiseSeconds = 0.3;

	void Update(double DeltaSeconds, bool bRaised)
	{
		if (!FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0.0)
		{
			return;
		}
		const double Step = DeltaSeconds / FMath::Max(RaiseSeconds, KINDA_SMALL_NUMBER);
		Alpha = FMath::Clamp(Alpha + (bRaised ? Step : -Step), 0.0, 1.0);
	}

	double Eased() const
	{
		return FMath::SmoothStep(0.0, 1.0, Alpha);
	}

	bool IsVisible() const
	{
		return Alpha > 0.0;
	}

	static FTransform Blend(const FTransform& Rest, const FTransform& Raised, double A)
	{
		const double T = FMath::Clamp(A, 0.0, 1.0);
		return FTransform(FQuat::Slerp(Rest.GetRotation(), Raised.GetRotation(), T),
			FMath::Lerp(Rest.GetLocation(), Raised.GetLocation(), T));
	}

	/** Pose from the elbow position, the forearm direction (elbow to fingers) and the back-of-wrist direction. */
	static FTransform Pose(const FVector& Elbow, const FVector& Forearm, const FVector& BackOfWrist)
	{
		return FTransform(FRotationMatrix::MakeFromXZ(Forearm.GetSafeNormal(), BackOfWrist.GetSafeNormal()).ToQuat(), Elbow);
	}
};
