#include "Narrative/Act2StateMachine.h"

void UAct2StateMachine::OnDayChanged(int32 Day)
{
	if (State == EAct2State::Act1 && Day >= 21)
	{
		State = EAct2State::SupplyBoatOverdue;
	}
	if (State == EAct2State::SupplyBoatOverdue && Day >= 40)
	{
		State = EAct2State::DistressIntercepted;
	}
}

bool UAct2StateMachine::ChooseBroadcast()
{
	if (State != EAct2State::DistressIntercepted)
	{
		return false;
	}
	State = EAct2State::A_Broadcasting;
	return true;
}

bool UAct2StateMachine::ChooseGoAlone()
{
	if (State != EAct2State::DistressIntercepted)
	{
		return false;
	}
	State = EAct2State::B_Preparing;
	return true;
}

bool UAct2StateMachine::CompletePrepTask(EPrepTask Task)
{
	if (State != EAct2State::B_Preparing)
	{
		return false;
	}
	uint8 Bit;
	switch (Task)
	{
	case EPrepTask::EmergencyDeco:
		Bit = 1;
		break;
	case EPrepTask::TetherHose600m:
		Bit = 2;
		break;
	case EPrepTask::DecoyBeacon:
		Bit = 4;
		break;
	default:
		return false;
	}
	if (PrepMask & Bit)
	{
		return false;
	}
	PrepMask |= Bit;
	if (PrepMask == 7)
	{
		State = EAct2State::B_Swim;
	}
	return true;
}

bool UAct2StateMachine::Advance(FName Event)
{
	struct FTransition
	{
		EAct2State From;
		FName Event;
		EAct2State To;
	};
	static const FTransition Transitions[] = {
		{EAct2State::A_Broadcasting, TEXT("FamilyArrived"), EAct2State::A_FamilyDocked},
		{EAct2State::A_FamilyDocked, TEXT("HordeArrived"), EAct2State::A_Siege48h},
		{EAct2State::A_Siege48h, TEXT("Scuttle"), EAct2State::A_EndingScuttled},
		{EAct2State::B_Swim, TEXT("ReachedMarina"), EAct2State::B_Siphon},
		{EAct2State::B_Siphon, TEXT("FuelSiphoned"), EAct2State::B_Return},
		{EAct2State::B_Return, TEXT("Repressurised"), EAct2State::B_EndingAlone}};
	for (const FTransition& Transition : Transitions)
	{
		if (State == Transition.From && Event == Transition.Event)
		{
			State = Transition.To;
			SiegeSeconds = State == EAct2State::A_Siege48h ? 48.0 * 3600.0 : 0.0;
			return true;
		}
	}
	return false;
}

void UAct2StateMachine::NotifyDeath()
{
	State = EAct2State::Dead;
	SiegeSeconds = 0.0;
}

void UAct2StateMachine::TickSiege(double GameSeconds)
{
	if (State != EAct2State::A_Siege48h || !FMath::IsFinite(GameSeconds) || GameSeconds <= 0.0)
	{
		return;
	}
	SiegeSeconds = FMath::Max(0.0, SiegeSeconds - GameSeconds);
	if (SiegeSeconds == 0.0)
	{
		State = EAct2State::A_EndingRescued;
	}
}
