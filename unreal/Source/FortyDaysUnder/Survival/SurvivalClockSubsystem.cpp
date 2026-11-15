#include "Survival/SurvivalClockSubsystem.h"
#include "Engine/GameInstance.h"

// Ticking starts in Initialize, never from the constructor (UObjects may be built off the game thread).
USurvivalClockSubsystem::USurvivalClockSubsystem()
	: FTickableGameObject(ETickableTickType::Never)
{
}

double USurvivalClockSubsystem::GetGameSeconds() const
{
	return SimulatedSeconds;
}

int32 USurvivalClockSubsystem::GetDay() const
{
	return 1 + FMath::FloorToInt32(SimulatedSeconds / 86400.0);
}

void USurvivalClockSubsystem::AdvanceBy(double GameSeconds)
{
	if (!(GameSeconds > 0.0) || !FMath::IsFinite(GameSeconds))
	{
		return;
	}
	CarriedSeconds += GameSeconds;
	while (CarriedSeconds >= StepSeconds)
	{
		CarriedSeconds -= StepSeconds;
		SimulatedSeconds += StepSeconds;
		OnSimStep.Broadcast(StepSeconds);
	}
}

void USurvivalClockSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	SetTickableTickType(ETickableTickType::Always);
}

void USurvivalClockSubsystem::Deinitialize()
{
	SetTickableTickType(ETickableTickType::Never);
	Super::Deinitialize();
}

void USurvivalClockSubsystem::Tick(float DeltaTime)
{
	if (TimeScale > 0.0)
	{
		AdvanceBy(DeltaTime * TimeScale);
	}
}

UWorld* USurvivalClockSubsystem::GetTickableGameObjectWorld() const
{
	// Tick with the game world so pause and PIE apply; null (e.g. in tests) means no world.
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetWorld() : nullptr;
}

TStatId USurvivalClockSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USurvivalClockSubsystem, STATGROUP_Tickables);
}
