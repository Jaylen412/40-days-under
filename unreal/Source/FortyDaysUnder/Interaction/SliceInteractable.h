#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/SliceInteractionModel.h"
#include "SliceInteractable.generated.h"

class AOpeningSliceActor;
class UBoxComponent;
class UPointLightComponent;

/**
 * A thing in the level the player uses, looks at or holds to complete one opening-slice beat.
 * It is live only while its beat is the director's current beat. Feedback is in-world only:
 * a beacon light that brightens with progress (no screen HUD).
 */
UCLASS()
class FORTYDAYSUNDER_API ASliceInteractable : public AActor
{
	GENERATED_BODY()

public:
	ASliceInteractable();

	/** What the player's view trace has to hit. Collides only while the interaction is live. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UBoxComponent> TraceVolume;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UPointLightComponent> Beacon;

	/** Beat id from opening_slice.json during which this is live. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName Beat;
	/** Completion event sent to the director, matching that beat's completeOnEvent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName Event;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	ESliceInteractionKind Kind = ESliceInteractionKind::Use;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	ESliceInput Input = ESliceInput::Interact;
	/** False for actions with no object to look at, such as getting up or raising the wrist. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bRequiresFocus = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "0.0"))
	double RequiredSeconds = 1.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "0.0"))
	double MaxDistanceCm = 250.0;

	/** Found automatically at BeginPlay when left empty. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TObjectPtr<AOpeningSliceActor> Slice;

	/** Shown or hidden when the beat begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Effects")
	TArray<TObjectPtr<AActor>> ShowOnActivate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Effects")
	TArray<TObjectPtr<AActor>> HideOnActivate;
	/** Shown or hidden when the beat ends, whether the player completed it or it timed out. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Effects")
	TArray<TObjectPtr<AActor>> ShowOnComplete;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Effects")
	TArray<TObjectPtr<AActor>> HideOnComplete;
	/** Turned about its local X axis as progress goes from 0 to 1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Effects")
	TObjectPtr<AActor> SpinActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Effects")
	double SpinDegrees = 0.0;

	/** Live means its beat is current and it has not fired. */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsLive() const;
	UFUNCTION(BlueprintPure, Category = "Interaction")
	double GetProgress01() const
	{
		return Model.Progress01(RequiredSeconds);
	}
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasFired() const
	{
		return Model.bFired;
	}

	/** Called by the player's interactor every frame while live. */
	void Advance(double DeltaSeconds, bool bFocused, bool bHeld, bool bPressed);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	FSliceInteractionModel Model;
	bool bWasLive = false;
	bool bCompleted = false;
	FQuat SpinBase = FQuat::Identity;

	void OnBecameLive();
	void OnCompleted();
	void ApplySpin(double Alpha);
	static void SetShown(const TArray<TObjectPtr<AActor>>& Actors, bool bShown);
};
