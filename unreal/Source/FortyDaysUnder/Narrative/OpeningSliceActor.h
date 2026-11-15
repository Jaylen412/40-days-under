#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OpeningSliceActor.generated.h"

class ALevelSequenceActor;
class UIntroFootageComponent;
class USliceDirector;

/**
 * Runs the Day 1 opening slice in a level: owns a USliceDirector loaded from JSON and keeps a
 * level sequence (character animation, props, camera cuts) on the director's current beat.
 */
UCLASS()
class FORTYDAYSUNDER_API AOpeningSliceActor : public AActor
{
	GENERATED_BODY()

public:
	AOpeningSliceActor();

	/** Sequence laid out with the beats back to back, in JSON order, each as long as its timeout. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	TObjectPtr<ALevelSequenceActor> SequenceActor;

	/** Slice file, relative to the project Content directory. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	FString SliceFile = TEXT("Data/Narrative/opening_slice.json");

	/** How far the sequence may sit from a beat's start before it is jumped there. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (ClampMin = "0.0"))
	double ResyncToleranceSeconds = 0.25;

	/** Higgsfield clips shown full screen before the slice starts; the director loads when they end. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
	TObjectPtr<UIntroFootageComponent> IntroFootage;

	UFUNCTION(BlueprintPure, Category = "Narrative")
	USliceDirector* GetDirector() const
	{
		return Director;
	}

	/** Forward a gameplay event (for example "WokeUp") to the director. True if it completed the current beat. */
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	bool NotifyEvent(FName Event);

	/** Sequence playback position in seconds, or -1 without a sequence. */
	UFUNCTION(BlueprintPure, Category = "Narrative")
	double GetSequenceSeconds() const;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<USliceDirector> Director;

	UFUNCTION()
	void HandleBeatStarted(FName BeatId);

	UFUNCTION()
	void StartSlice();
};
