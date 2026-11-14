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
	/**
	 * -1 sinks, +1 floats, 0 neutral. Applied by this component as a vertical drift of
	 * NetBuoyancy * BaseSwimSpeed cm/s. It is independent of the engine's own Buoyancy
	 * property, which only scales gravity in the engine's PhysSwimming; that function is
	 * not run here, and Buoyancy is set to 1.0 (neutral) in the constructor for clarity.
	 */
	UPROPERTY(EditAnywhere) float NetBuoyancy = 0.0f;
	UPROPERTY(EditAnywhere) float BaseSwimSpeed = 180.0f;     // cm/s

	/**
	 * When true the component behaves as fully submerged even with no water APhysicsVolume
	 * in the level (the default map has none). Driven by the character: true outside the lab.
	 */
	UPROPERTY(EditAnywhere) bool bForceSubmerged = false;

	/** Base x lerp(0.4, 1, Stamina/100); halved at Hypoxic or worse. */
	float GetEffectiveSwimSpeed(float Stamina, EAirStage Air) const;

	/** Replaces the engine swim step; current and buoyancy are part of Velocity. */
	virtual void PhysSwimming(float DeltaTime, int32 Iterations) override;
	virtual bool IsInWater() const override;
	virtual float ImmersionDepth() const override;
};
