#include "Character/UnderwaterMovementComponent.h"

UUnderwaterMovementComponent::UUnderwaterMovementComponent()
{
	DefaultWaterMovementMode = MOVE_Swimming;
	GravityScale = 0.0f;
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

void UUnderwaterMovementComponent::PhysSwimming(float DeltaTime, int32 Iterations)
{
	Super::PhysSwimming(DeltaTime, Iterations);

	if (DeltaTime <= 0.0f || !UpdatedComponent)
	{
		return;
	}

	// Drift: the water current plus buoyancy (scaled by base swim speed) displace the body.
	const FVector Drift = WaterCurrentCmPerSec + FVector(0.0f, 0.0f, NetBuoyancy * BaseSwimSpeed);
	if (!Drift.IsNearlyZero())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(Drift * DeltaTime, UpdatedComponent->GetComponentQuat(), true, Hit);
		if (Hit.IsValidBlockingHit())
		{
			SlideAlongSurface(Drift * DeltaTime, 1.0f - Hit.Time, Hit.Normal, Hit, true);
		}
	}
}
