#include "Character/UnderwaterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PhysicsVolume.h"

UUnderwaterMovementComponent::UUnderwaterMovementComponent()
{
	DefaultWaterMovementMode = MOVE_Swimming;
	Buoyancy = 1.0f;   // engine value: neutral. Our NetBuoyancy is separate (see header).
	MaxSwimSpeed = BaseSwimSpeed;
}

float UUnderwaterMovementComponent::GetEffectiveSwimSpeed(float Stamina, EAirStage Air) const
{
	const float Alpha = FMath::Clamp(Stamina / 100.0f, 0.0f, 1.0f);
	float Speed = BaseSwimSpeed * FMath::Lerp(0.4f, 1.0f, Alpha);
	if (Air >= EAirStage::Hypoxic)
	{
		Speed *= 0.5f;
	}
	return Speed;
}

bool UUnderwaterMovementComponent::IsInWater() const
{
	if (bForceSubmerged)
	{
		return true;
	}
	if (!UpdatedComponent && !GetWorld())
	{
		return false;   // no world (CDO / tests): Super would dereference the default volume
	}
	return Super::IsInWater();
}

float UUnderwaterMovementComponent::ImmersionDepth() const
{
	return bForceSubmerged ? 1.0f : Super::ImmersionDepth();
}

void UUnderwaterMovementComponent::PhysSwimming(float DeltaTime, int32 Iterations)
{
	// The engine's PhysSwimming checks GetPhysicsVolume()->bWaterVolume directly (to compute
	// the velocity, and to drop to MOVE_Falling), which a forced-submerged character without a
	// water volume would fail. So the swim step is reimplemented here. Together with the
	// IsInWater/ImmersionDepth overrides this defeats every volume check the engine makes:
	// SetDefaultMovementMode, SetPostLandedPhysics, StartFalling-style mode selection
	// (IsInWater), ImmersionDepth (bWaterVolume), and PhysSwimming/Swim (bWaterVolume).
	if (DeltaTime < MIN_TICK_TIME || !UpdatedComponent || !CharacterOwner)
	{
		return;
	}

	RestorePreAdditiveRootMotionVelocity();

	// Ambient drift in cm/s: the water current plus buoyancy.
	const FVector Drift = WaterCurrentCmPerSec + FVector(0.0f, 0.0f, NetBuoyancy * BaseSwimSpeed);
	const FVector OldLocation = UpdatedComponent->GetComponentLocation();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		// Velocity is ground-frame and includes Drift. Friction and acceleration act on the
		// water-relative velocity, so strip the drift, integrate, and put it back. Everything
		// here is a rate (cm/s) multiplied by DeltaTime, so it is frame-rate independent.
		const float Friction = 0.5f * GetPhysicsVolume()->FluidFriction;
		Velocity -= Drift;
		CalcVelocity(DeltaTime, Friction, true, GetMaxBrakingDeceleration());
		Velocity += Drift;
	}

	ApplyRootMotionToVelocity(DeltaTime);

	const FVector Adjusted = Velocity * DeltaTime;
	FHitResult Hit(1.0f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);
	float MovedTime = DeltaTime;
	if (Hit.Time < 1.0f)
	{
		HandleImpact(Hit, DeltaTime, Adjusted);
		SlideAlongSurface(Adjusted, 1.0f - Hit.Time, Hit.Normal, Hit, true);
	}

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && !bJustTeleported)
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / MovedTime;
	}

	// Mirror the engine: swimming without water (and not forced) falls.
	if (IsSwimming() && !IsInWater())
	{
		SetMovementMode(MOVE_Falling);
		StartNewPhysics(0.0f, Iterations + 1);
	}
}
