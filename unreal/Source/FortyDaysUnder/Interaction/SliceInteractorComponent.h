#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Interaction/SliceInteractionModel.h"
#include "SliceInteractorComponent.generated.h"

class ASliceInteractable;

/** Player side of the opening-slice interactions: finds what the view is on and feeds input to live interactables. */
UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class FORTYDAYSUNDER_API USliceInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USliceInteractorComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "0.0"))
	double TraceDistanceCm = 1500.0;

	/** Input state, set by the owning character's bindings. A false-to-true change counts as one press. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInputHeld(ESliceInput Input, bool bHeld);

	/** The live interactable the view is on and within its reach, if any. */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	ASliceInteractable* GetFocused() const
	{
		return Focused.Get();
	}

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool Held[2] = {false, false};
	bool Pressed[2] = {false, false};
	TWeakObjectPtr<ASliceInteractable> Focused;

	ASliceInteractable* FindFocused() const;
};
