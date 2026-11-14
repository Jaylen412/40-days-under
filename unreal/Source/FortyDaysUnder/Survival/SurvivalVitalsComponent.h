#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Simulation/AirSupplyModel.h"
#include "Simulation/BodyModel.h"
#include "Simulation/BuhlmannModel.h"
#include "Simulation/VitalsSnapshot.h"
#include "SurvivalVitalsComponent.generated.h"

class USurvivalClockSubsystem;

/**
 * Owns the survival models for one diver, steps them on the survival clock and
 * publishes the result as an FVitalsSnapshot.
 */
UCLASS(ClassGroup=(Survival), meta=(BlueprintSpawnableComponent))
class FORTYDAYSUNDER_API USurvivalVitalsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurvivalVitalsComponent();

	UFUNCTION(BlueprintCallable) const FVitalsSnapshot& GetSnapshot() const;
	UFUNCTION(BlueprintCallable) void SetDepthMetres(double DepthM);   // sets ambient pressure, breathing air
	UFUNCTION(BlueprintCallable) void BeginDecoCycle();                // airlock: ambient follows DecoAmbientBar, gas DecoGasFN2
	UFUNCTION(BlueprintCallable) void AbortDecoCycle();
	UFUNCTION(BlueprintCallable) double AddFuel(double Litres);
	UFUNCTION(BlueprintCallable) bool EatMeal();
	UFUNCTION(BlueprintCallable) bool Drink(double Litres);

	void SimStep(double Seconds);       // public so tests drive it without a world

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVitalsChanged, const FVitalsSnapshot&, Snapshot);
	UPROPERTY(BlueprintAssignable) FOnVitalsChanged OnVitalsChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDied);
	UPROPERTY(BlueprintAssignable) FOnDied OnDied;   // fires once

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void SubStep(double Seconds);
	void RefreshSnapshot();
	void PublishChanges();

	FBuhlmannModel Tissues;
	FAirSupplyModel Air;
	FBodyModel Body;

	double GameSeconds = 0.0;
	double AmbientBar = 0.0;
	bool bInDecoCycle = false;
	double DecoElapsedSeconds = 0.0;
	bool bDiedBroadcast = false;

	FVitalsSnapshot Snapshot;

	TWeakObjectPtr<USurvivalClockSubsystem> Clock;
	FDelegateHandle SimStepHandle;
};
