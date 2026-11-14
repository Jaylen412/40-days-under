#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Input/DualSenseFeedbackProfile.h"
#include "Simulation/VitalsSnapshot.h"
#include "DualSenseFeedbackComponent.generated.h"

class UHapticFeedbackEffect_Base;
class USurvivalVitalsComponent;

/**
 * Applies the survival state to a DualSense: adaptive trigger, lightbar and pump haptics.
 * Every entry point is a silent no-op when no local player or input device is available.
 */
UCLASS(ClassGroup=(Input), meta=(BlueprintSpawnableComponent))
class FORTYDAYSUNDER_API UDualSenseFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDualSenseFeedbackComponent();

	UFUNCTION(BlueprintCallable) void SetTriggerAction(ETriggerAction Action, float Progress01);
	UFUNCTION(BlueprintCallable) void SetHordeProximity(float Proximity01);

	/** Bound to the owner's USurvivalVitalsComponent::OnVitalsChanged. */
	UFUNCTION() void HandleVitalsChanged(const FVitalsSnapshot& Snapshot);

	/** Played on the left grip in time with the pump heartbeat. Asset assigned later. */
	UPROPERTY(EditAnywhere) TObjectPtr<UHapticFeedbackEffect_Base> PumpThumpEffect;

	bool HasSupportedDevice() const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ApplyTrigger();
	void ApplyLightbar(double RealTimeSeconds);
	void UpdatePumpHaptic(float DeltaTime);

	FVitalsSnapshot Snapshot;
	ETriggerAction Action = ETriggerAction::None;
	float Progress01 = 0.0f;
	float HordeProximity01 = 0.0f;

	float PumpPhase = 0.0f;
	FColor LastLightbar = FColor::Black;
	bool bLightbarSent = false;

	TWeakObjectPtr<USurvivalVitalsComponent> BoundVitals;
};
