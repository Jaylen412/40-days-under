#pragma once

#include "CoreMinimal.h"

/** One pre-rendered clip and the slice beat it shows. File is relative to the Content directory. */
struct FFootageClip
{
	FName BeatId;
	FString File;
};

/**
 * Ordered list of pre-rendered clips (the Higgsfield realism footage) and which one is playing.
 * Plain struct so it can be tested without a media player.
 */
struct FORTYDAYSUNDER_API FFootagePlaylist
{
	/** Reads {"clips":[{"beat":"wake_pump","file":"Movies/Higgsfield/01_wake_pump.mp4"}, ...]}. */
	static bool ParseJson(const FString& Json, TArray<FFootageClip>& OutClips, FString& OutError);

	/** Keeps the clips whose file exists, in order. Exists receives the path joined to BaseDir. */
	static TArray<FFootageClip> KeepExisting(
		const TArray<FFootageClip>& Clips, const FString& BaseDir, TFunctionRef<bool(const FString&)> Exists);

	void Reset(TArray<FFootageClip> InClips)
	{
		Clips = MoveTemp(InClips);
		Index = 0;
	}

	const FFootageClip* Current() const
	{
		return Clips.IsValidIndex(Index) ? &Clips[Index] : nullptr;
	}

	/** Moves to the next clip. False once the list is finished. */
	bool Advance()
	{
		if (Index < Clips.Num())
		{
			++Index;
		}
		return Index < Clips.Num();
	}

	void SkipAll()
	{
		Index = Clips.Num();
	}

	bool IsFinished() const
	{
		return Index >= Clips.Num();
	}

	int32 Num() const
	{
		return Clips.Num();
	}

	int32 CurrentIndex() const
	{
		return Index;
	}

private:
	TArray<FFootageClip> Clips;
	int32 Index = 0;
};
