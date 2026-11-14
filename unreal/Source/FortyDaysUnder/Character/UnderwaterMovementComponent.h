#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Simulation/VitalsSnapshot.h"
#include "UnderwaterMovementComponent.generated.h"

/** Swimming movement with an ambient water current, net buoyancy and stamina/air-limited speed. */
UCLASS()
class FORTYDAYSUNDER_API UUnderwaterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UUnderwaterMovementComponent();

	UPROPERTY(EditAnywhere) FVector WaterCurrentCmPerSec = FVector::ZeroVector;
	UPROPERTY(EditAnywhere) float NetBuoyancy = 0.0f;         // -1 sinks, +1 floats, 0 neutral
	UPROPERTY(EditAnywhere) float BaseSwimSpeed = 180.0f;     // cm/s

	/** Base x lerp(0.4, 1, Stamina/100); halved at Hypoxic or worse. */
	float GetEffectiveSwimSpeed(float Stamina, EAirStage Air) const;

	virtual void PhysSwimming(float DeltaTime, int32 Iterations) override; // adds current and buoyancy
};
