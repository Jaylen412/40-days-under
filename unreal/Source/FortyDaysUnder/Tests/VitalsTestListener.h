#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Simulation/VitalsSnapshot.h"
#include "VitalsTestListener.generated.h"

/** Test-only receiver for the vitals component's dynamic delegates (they cannot bind lambdas). */
UCLASS(Transient, NotBlueprintable)
class UVitalsTestListener : public UObject
{
	GENERATED_BODY()

public:
	int32 DiedCount = 0;
	int32 ChangedCount = 0;

	UFUNCTION()
	void HandleDied() { ++DiedCount; }

	UFUNCTION()
	void HandleVitalsChanged(const FVitalsSnapshot& Snapshot) { ++ChangedCount; }
};
