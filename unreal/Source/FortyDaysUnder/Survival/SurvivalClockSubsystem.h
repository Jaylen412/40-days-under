#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "SurvivalClockSubsystem.generated.h"

/**
 * Deterministic survival clock. Scales real time into game time and emits fixed
 * StepSeconds simulation steps, carrying any remainder to the next advance.
 */
UCLASS()
class FORTYDAYSUNDER_API USurvivalClockSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	USurvivalClockSubsystem();

	static constexpr double StepSeconds = 60.0;

	UPROPERTY(EditAnywhere)
	double TimeScale = 60.0;   // game seconds per real second; 0 pauses

	double GetGameSeconds() const;
	int32  GetDay() const;                              // 1 + floor(GameSeconds / 86400)
	void   AdvanceBy(double GameSeconds);               // runs whole StepSeconds steps, carries remainder

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnSimStep, double /*StepSeconds*/);
	FOnSimStep OnSimStep;

	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;

private:
	double SimulatedSeconds = 0.0;
	double CarriedSeconds = 0.0;
};
