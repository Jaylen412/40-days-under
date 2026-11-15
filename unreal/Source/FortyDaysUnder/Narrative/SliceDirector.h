#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SliceDirector.generated.h"

USTRUCT(BlueprintType)
struct FORTYDAYSUNDER_API FSliceBeat
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Narrative")
	FName Id;
	UPROPERTY(BlueprintReadOnly, Category = "Narrative")
	FString Speaker;
	UPROPERTY(BlueprintReadOnly, Category = "Narrative")
	FString Line;
	UPROPERTY(BlueprintReadOnly, Category = "Narrative")
	FName CompleteOnEvent;
	UPROPERTY(BlueprintReadOnly, Category = "Narrative")
	double TimeoutSeconds = 0.0;
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSliceBeatStarted, FName, BeatId);

/** Data-driven opening sequence. Time is real seconds; excess time carries across beats. */
UCLASS(BlueprintType)
class FORTYDAYSUNDER_API USliceDirector : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	bool LoadFromJsonString(const FString& Json, FString& OutError);
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	bool LoadFromFile(const FString& Path, FString& OutError);
	UFUNCTION(BlueprintPure, Category = "Narrative")
	int32 NumBeats() const
	{
		return Beats.Num();
	}
	UFUNCTION(BlueprintPure, Category = "Narrative")
	FName CurrentBeatId() const;
	UFUNCTION(BlueprintPure, Category = "Narrative")
	FString CurrentSpeaker() const;
	UFUNCTION(BlueprintPure, Category = "Narrative")
	FString CurrentLine() const;
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	bool NotifyEvent(FName Event);
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void Tick(double RealSeconds);
	UFUNCTION(BlueprintPure, Category = "Narrative")
	bool IsFinished() const
	{
		return !Beats.IsEmpty() && CurrentIndex >= Beats.Num();
	}
	/** Seconds from the start of the slice to the start of this beat if every earlier beat runs to its timeout; -1 if unknown. */
	UFUNCTION(BlueprintPure, Category = "Narrative")
	double BeatStartSeconds(FName BeatId) const;
	/** Sum of every beat's timeout. */
	UFUNCTION(BlueprintPure, Category = "Narrative")
	double TotalSeconds() const;
	UPROPERTY(BlueprintAssignable, Category = "Narrative")
	FOnSliceBeatStarted OnBeatStarted;

private:
	UPROPERTY()
	TArray<FSliceBeat> Beats;
	int32 CurrentIndex = 0;
	double RemainingSeconds = 0.0;
	// Detect a synchronous listener replacing/advancing the sequence during a tick.
	uint64 Revision = 0;
	void StartCurrentBeat();
	void AdvanceBeat();
};
