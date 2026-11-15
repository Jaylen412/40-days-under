#pragma once

#include "CoreMinimal.h"
#include "SliceInteractionModel.generated.h"

/** How the player completes an interaction. */
UENUM(BlueprintType)
enum class ESliceInteractionKind : uint8
{
	Use,  // press the input once while focused
	Gaze, // keep it in view for RequiredSeconds, no input
	Hold  // keep the input held while focused for RequiredSeconds
};

/** Which player input an interaction listens to. */
UENUM(BlueprintType)
enum class ESliceInput : uint8
{
	Interact,
	Watch
};

/** Plain progress rules for one interaction, testable without a world. Fires at most once. */
struct FSliceInteractionModel
{
	double ProgressSeconds = 0.0;
	bool bFired = false;

	/** Advance by Dt seconds. Returns true only on the update that completes the interaction. */
	bool Update(ESliceInteractionKind Kind, bool bFocused, bool bHeld, bool bPressed, double Dt, double RequiredSeconds)
	{
		if (bFired)
		{
			return false;
		}
		if (!FMath::IsFinite(Dt) || Dt < 0.0)
		{
			Dt = 0.0;
		}
		if (!bFocused)
		{
			// Looking away abandons a gaze or a hold; nothing can complete unfocused.
			ProgressSeconds = 0.0;
			return false;
		}
		switch (Kind)
		{
		case ESliceInteractionKind::Use:
			bFired = bPressed;
			break;
		case ESliceInteractionKind::Gaze:
			ProgressSeconds += Dt;
			bFired = ProgressSeconds >= RequiredSeconds;
			break;
		case ESliceInteractionKind::Hold:
			// Letting go pauses the effort; it does not undo it.
			if (bHeld)
			{
				ProgressSeconds += Dt;
				bFired = ProgressSeconds >= RequiredSeconds;
			}
			break;
		}
		return bFired;
	}

	/** 0 to 1; a fired interaction reads 1. */
	double Progress01(double RequiredSeconds) const
	{
		if (bFired)
		{
			return 1.0;
		}
		return RequiredSeconds > 0.0 ? FMath::Clamp(ProgressSeconds / RequiredSeconds, 0.0, 1.0) : 0.0;
	}
};
