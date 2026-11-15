#include "Survival/SurvivalVitalsComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Survival/SurvivalClockSubsystem.h"

namespace
{
	constexpr double SecondsPerHour = 3600.0;
	constexpr double VitalsSecondsPerDay = 86400.0;
	constexpr double DecoCycleSeconds = FBuhlmannModel::DecoCycleHours * SecondsPerHour;
}

USurvivalVitalsComponent::USurvivalVitalsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Christian has lived at Lab 04 long enough to be saturated at 15 m on air.
	Tissues.ResetToSaturationAt(FBuhlmannModel::LabDepthM, FBuhlmannModel::AirFN2);
	AmbientBar = FBuhlmannModel::AmbientBarAtDepth(FBuhlmannModel::LabDepthM);
	RefreshSnapshot();
}

void USurvivalVitalsComponent::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	if (USurvivalClockSubsystem* ClockSubsystem = GameInstance ? GameInstance->GetSubsystem<USurvivalClockSubsystem>() : nullptr)
	{
		Clock = ClockSubsystem;
		SimStepHandle = ClockSubsystem->OnSimStep.AddUObject(this, &USurvivalVitalsComponent::SimStep);
	}
}

void USurvivalVitalsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (USurvivalClockSubsystem* ClockSubsystem = Clock.Get())
	{
		ClockSubsystem->OnSimStep.Remove(SimStepHandle);
	}
	Clock.Reset();
	SimStepHandle.Reset();

	Super::EndPlay(EndPlayReason);
}

const FVitalsSnapshot& USurvivalVitalsComponent::GetSnapshot() const
{
	return Snapshot;
}

void USurvivalVitalsComponent::SetDepthMetres(double DepthM)
{
	if (bInDecoCycle || !FMath::IsFinite(DepthM))
	{
		return; // the airlock schedule owns the pressure until it completes or is aborted
	}
	const double NewAmbient = FBuhlmannModel::AmbientBarAtDepth(DepthM);
	if (NewAmbient != AmbientBar)
	{
		AmbientBar = NewAmbient;
		PublishChanges();
	}
}

void USurvivalVitalsComponent::BeginDecoCycle()
{
	bInDecoCycle = true;
	DecoElapsedSeconds = 0.0;
	AmbientBar = FBuhlmannModel::DecoAmbientBar(0.0);
	PublishChanges();
}

void USurvivalVitalsComponent::AbortDecoCycle()
{
	if (!bInDecoCycle)
	{
		return;
	}
	// Pressure stays where the schedule left it until the next SetDepthMetres.
	bInDecoCycle = false;
	DecoElapsedSeconds = 0.0;
	PublishChanges();
}

double USurvivalVitalsComponent::AddFuel(double Litres)
{
	const double Accepted = Air.AddFuel(Litres);
	PublishChanges();
	return Accepted;
}

bool USurvivalVitalsComponent::EatMeal()
{
	const bool bAte = Body.EatMeal();
	PublishChanges();
	return bAte;
}

bool USurvivalVitalsComponent::Drink(double Litres)
{
	const bool bDrank = Body.Drink(Litres);
	PublishChanges();
	return bDrank;
}

void USurvivalVitalsComponent::SimStep(double Seconds)
{
	if (!(Seconds > 0.0) || !FMath::IsFinite(Seconds) || Body.IsDead())
	{
		return;
	}

	// Whole clock steps plus a remainder, so one large call matches many small ones.
	const double Step = USurvivalClockSubsystem::StepSeconds;
	const int64 WholeSteps = FMath::FloorToInt64(Seconds / Step);
	const double Remainder = Seconds - WholeSteps * Step;
	for (int64 i = 0; i < WholeSteps && !Body.IsDead(); ++i)
	{
		SubStep(Step);
	}
	if (Remainder > 0.0 && !Body.IsDead())
	{
		SubStep(Remainder);
	}

	PublishChanges();
}

void USurvivalVitalsComponent::SubStep(double Seconds)
{
	Air.Advance(Seconds);

	if (bInDecoCycle)
	{
		DecoElapsedSeconds = FMath::Min(DecoElapsedSeconds + Seconds, DecoCycleSeconds);
		AmbientBar = FBuhlmannModel::DecoAmbientBar(DecoElapsedSeconds / SecondsPerHour);
		Tissues.Advance(Seconds, AmbientBar, FBuhlmannModel::DecoGasFN2);
		if (DecoElapsedSeconds >= DecoCycleSeconds)
		{
			// Cycle complete: stay at surface pressure, back on air.
			bInDecoCycle = false;
			DecoElapsedSeconds = 0.0;
		}
	}
	else
	{
		Tissues.Advance(Seconds, AmbientBar, FBuhlmannModel::AirFN2);
	}

	Body.Advance(Seconds, Air.IsPumpRunning(), Air.Stage(), Tissues.DcsSeverityAt(AmbientBar));
	GameSeconds += Seconds;
}

void USurvivalVitalsComponent::RefreshSnapshot()
{
	Snapshot.GameSeconds = GameSeconds;
	Snapshot.Day = 1 + FMath::FloorToInt32(GameSeconds / VitalsSecondsPerDay);
	Snapshot.FuelFraction = Air.FuelFraction();
	Snapshot.bPumpRunning = Air.IsPumpRunning();
	Snapshot.bPumpSputtering = Air.IsSputtering();
	Snapshot.Co2Pct = Air.Co2Pct;
	Snapshot.O2Pct = Air.O2Pct;
	Snapshot.AirStage = Air.Stage();
	Snapshot.AmbientBar = AmbientBar;
	Snapshot.CeilingBar = Tissues.CeilingBar();
	Snapshot.DcsSeverity = Tissues.DcsSeverityAt(AmbientBar);
	Snapshot.bInDecoCycle = bInDecoCycle;
	Snapshot.DecoHoursRemaining = bInDecoCycle ? (DecoCycleSeconds - DecoElapsedSeconds) / SecondsPerHour : 0.0;
	Snapshot.Health = Body.Health;
	Snapshot.Stamina = Body.Stamina;
	Snapshot.HydrationL = Body.HydrationL;
	Snapshot.CaloriesKcal = Body.CaloriesKcal;
	Snapshot.Isolation = Body.Isolation;
	Snapshot.bDead = Body.IsDead();
}

void USurvivalVitalsComponent::PublishChanges()
{
	RefreshSnapshot();
	OnVitalsChanged.Broadcast(Snapshot);
	if (Snapshot.bDead && !bDiedBroadcast)
	{
		bDiedBroadcast = true;
		OnDied.Broadcast();
	}
}
