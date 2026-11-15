#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Narrative/Act2StateMachine.h"
#include "Narrative/SliceDirector.h"
#include "Narrative/SlicePlaybackSync.h"
#include "UObject/StrongObjectPtr.h"
#include <limits>

#if WITH_DEV_AUTOMATION_TESTS
namespace NarrativeTest
{
constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
const FString TwoBeats = TEXT(
	R"({"beats":[{"id":"one","speaker":"C","line":"First","completeOnEvent":"Go","timeoutSeconds":2},{"id":"two","speaker":"S","line":"Second","completeOnEvent":"Done","timeoutSeconds":3}]})");
void Siege(UAct2StateMachine &M)
{
	M.OnDayChanged(40);
	M.ChooseBroadcast();
	M.Advance("FamilyArrived");
	M.Advance("HordeArrived");
}
void Prepare(UAct2StateMachine &M)
{
	M.CompletePrepTask(EPrepTask::EmergencyDeco);
	M.CompletePrepTask(EPrepTask::TetherHose600m);
	M.CompletePrepTask(EPrepTask::DecoyBeacon);
}
} // namespace NarrativeTest
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrativeDays, "FortyDaysUnder.Narrative.DayTriggers", NarrativeTest::Flags)
bool FNarrativeDays::RunTest(const FString &Parameters)
{
	TStrongObjectPtr<UAct2StateMachine> M(NewObject<UAct2StateMachine>());
	TestEqual(TEXT("initial"), M->GetState(), EAct2State::Act1);
	M->OnDayChanged(-1);
	M->OnDayChanged(20);
	TestEqual(TEXT("before overdue"), M->GetState(), EAct2State::Act1);
	M->OnDayChanged(21);
	TestEqual(TEXT("overdue"), M->GetState(), EAct2State::SupplyBoatOverdue);
	M->OnDayChanged(1);
	M->OnDayChanged(39);
	TestEqual(TEXT("no regression"), M->GetState(), EAct2State::SupplyBoatOverdue);
	M->OnDayChanged(40);
	TestEqual(TEXT("distress"), M->GetState(), EAct2State::DistressIntercepted);
	TStrongObjectPtr<UAct2StateMachine> Jump(NewObject<UAct2StateMachine>());
	Jump->OnDayChanged(100);
	TestEqual(TEXT("skipped days"), Jump->GetState(), EAct2State::DistressIntercepted);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrativeA, "FortyDaysUnder.Narrative.BranchA", NarrativeTest::Flags)
bool FNarrativeA::RunTest(const FString &Parameters)
{
	TStrongObjectPtr<UAct2StateMachine> M(NewObject<UAct2StateMachine>());
	NarrativeTest::Siege(*M);
	TestEqual(TEXT("siege"), M->GetState(), EAct2State::A_Siege48h);
	TestEqual(TEXT("48 hours"), M->SiegeHoursRemaining(), 48.0);
	M->TickSiege(-1);
	M->TickSiege(std::numeric_limits<double>::infinity());
	M->TickSiege(std::numeric_limits<double>::quiet_NaN());
	TestEqual(TEXT("bad deltas ignored"), M->SiegeHoursRemaining(), 48.0);
	M->TickSiege(47 * 3600);
	TestEqual(TEXT("one hour"), M->SiegeHoursRemaining(), 1.0);
	M->TickSiege(3600);
	TestEqual(TEXT("rescue"), M->GetState(), EAct2State::A_EndingRescued);
	TestFalse(TEXT("cannot scuttle after rescue"), M->Advance("Scuttle"));
	TStrongObjectPtr<UAct2StateMachine> S(NewObject<UAct2StateMachine>());
	NarrativeTest::Siege(*S);
	TestTrue(TEXT("scuttle"), S->Advance("Scuttle"));
	S->TickSiege(1e300);
	TestEqual(TEXT("scuttle terminal"), S->GetState(), EAct2State::A_EndingScuttled);
	TStrongObjectPtr<UAct2StateMachine> Huge(NewObject<UAct2StateMachine>());
	NarrativeTest::Siege(*Huge);
	Huge->TickSiege(std::numeric_limits<double>::max());
	TestEqual(TEXT("huge finite delta rescues"), Huge->GetState(), EAct2State::A_EndingRescued);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrativeB, "FortyDaysUnder.Narrative.BranchB", NarrativeTest::Flags)
bool FNarrativeB::RunTest(const FString &Parameters)
{
	TStrongObjectPtr<UAct2StateMachine> M(NewObject<UAct2StateMachine>());
	M->OnDayChanged(40);
	TestTrue(TEXT("choose alone"), M->ChooseGoAlone());
	TestFalse(TEXT("no changing branch"), M->ChooseBroadcast());
	TestFalse(TEXT("invalid enum"), M->CompletePrepTask(static_cast<EPrepTask>(255)));
	TestTrue(TEXT("first prep"), M->CompletePrepTask(EPrepTask::EmergencyDeco));
	TestFalse(TEXT("duplicate prep"), M->CompletePrepTask(EPrepTask::EmergencyDeco));
	M->CompletePrepTask(EPrepTask::DecoyBeacon);
	TestEqual(TEXT("still preparing"), M->GetState(), EAct2State::B_Preparing);
	M->CompletePrepTask(EPrepTask::TetherHose600m);
	TestEqual(TEXT("swim"), M->GetState(), EAct2State::B_Swim);
	TestFalse(TEXT("wrong event"), M->Advance("FuelSiphoned"));
	TestTrue(TEXT("marina"), M->Advance("ReachedMarina"));
	TestEqual(TEXT("siphon"), M->GetState(), EAct2State::B_Siphon);
	TestTrue(TEXT("fuel"), M->Advance("FuelSiphoned"));
	TestEqual(TEXT("return"), M->GetState(), EAct2State::B_Return);
	TestTrue(TEXT("repressurise"), M->Advance("Repressurised"));
	TestEqual(TEXT("alone ending"), M->GetState(), EAct2State::B_EndingAlone);
	M->OnDayChanged(40);
	TestFalse(TEXT("terminal prep"), M->CompletePrepTask(EPrepTask::EmergencyDeco));
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrativeRejects, "FortyDaysUnder.Narrative.RejectsBadInput", NarrativeTest::Flags)
bool FNarrativeRejects::RunTest(const FString &Parameters)
{
	TStrongObjectPtr<UAct2StateMachine> M(NewObject<UAct2StateMachine>());
	TestFalse(TEXT("early broadcast"), M->ChooseBroadcast());
	TestFalse(TEXT("early alone"), M->ChooseGoAlone());
	TestFalse(TEXT("early event"), M->Advance("FamilyArrived"));
	TestFalse(TEXT("early prep"), M->CompletePrepTask(EPrepTask::EmergencyDeco));
	TestEqual(TEXT("no mutation"), M->GetState(), EAct2State::Act1);
	TStrongObjectPtr<USliceDirector> D(NewObject<USliceDirector>());
	FString Error;
	TestEqual(TEXT("unloaded id"), D->CurrentBeatId(), NAME_None);
	TestFalse(TEXT("unloaded not finished"), D->IsFinished());
	TestTrue(TEXT("valid load"), D->LoadFromJsonString(NarrativeTest::TwoBeats, Error));
	TestFalse(TEXT("wrong event"), D->NotifyEvent("Done"));
	D->Tick(1);
	TArray<FString> Bad = {
		TEXT(""), TEXT("{"), TEXT("{}"), TEXT("{\"beats\":[]}"), TEXT("{\"beats\":[3]}"), TEXT("{\"beats\":true}")};
	for (const FString &Field :
		 {FString(TEXT("id")), FString(TEXT("speaker")), FString(TEXT("line")), FString(TEXT("completeOnEvent"))})
	{
		FString Value = NarrativeTest::TwoBeats;
		int32 Start = Value.Find(TEXT("\"") + Field + TEXT("\":\""));
		int32 End = Value.Find(TEXT("\""), ESearchCase::CaseSensitive, ESearchDir::FromStart, Start + Field.Len() + 4);
		Value.RemoveAt(Start + Field.Len() + 3, End - (Start + Field.Len() + 3) + 1);
		Value.InsertAt(Start + Field.Len() + 3, TEXT("42"));
		Bad.Add(Value);
	}
	Bad.Add(NarrativeTest::TwoBeats.Replace(TEXT("\"two\""), TEXT("\"one\"")));
	Bad.Add(NarrativeTest::TwoBeats.Replace(TEXT("\"Go\""), TEXT("\"\"")));
	Bad.Add(NarrativeTest::TwoBeats.Replace(TEXT("\"one\""), TEXT("\"None\"")));
	for (const TCHAR *Time : {TEXT("0"), TEXT("-1"), TEXT("\"2\""), TEXT("1e999")})
	{
		Bad.Add(NarrativeTest::TwoBeats.Replace(TEXT("\"timeoutSeconds\":2"),
												*(FString(TEXT("\"timeoutSeconds\":")) + Time)));
	}
	for (const FString &Json : Bad)
	{
		TestFalse(TEXT("bad load rejected"), D->LoadFromJsonString(Json, Error));
		TestFalse(TEXT("error supplied"), Error.IsEmpty());
		TestEqual(TEXT("progress retained"), D->CurrentBeatId(), FName("one"));
	}
	TestFalse(TEXT("missing file"), D->LoadFromFile(FPaths::ProjectContentDir() / TEXT("missing.json"), Error));
	D->Tick(-1);
	D->Tick(0);
	D->Tick(std::numeric_limits<double>::infinity());
	D->Tick(std::numeric_limits<double>::quiet_NaN());
	TestEqual(TEXT("invalid ticks ignored"), D->CurrentBeatId(), FName("one"));
	D->Tick(1);
	TestEqual(TEXT("elapsed retained across rejected loads"), D->CurrentBeatId(), FName("two"));
	TestTrue(TEXT("matching event"), D->NotifyEvent("Done"));
	TestTrue(TEXT("finished"), D->IsFinished());
	TestFalse(TEXT("finished event rejected"), D->NotifyEvent("Done"));
	D->LoadFromJsonString(NarrativeTest::TwoBeats, Error);
	D->Tick(4);
	TestEqual(TEXT("overflow carries"), D->CurrentBeatId(), FName("two"));
	D->Tick(1);
	TestTrue(TEXT("carried time completes"), D->IsFinished());
	D->LoadFromJsonString(NarrativeTest::TwoBeats, Error);
	D->Tick(std::numeric_limits<double>::max());
	TestTrue(TEXT("huge time finishes safely"), D->IsFinished());
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrativeSlice, "FortyDaysUnder.Narrative.SliceFile", NarrativeTest::Flags)
bool FNarrativeSlice::RunTest(const FString &Parameters)
{
	TStrongObjectPtr<USliceDirector> D(NewObject<USliceDirector>());
	FString Error;
	TestTrue(TEXT("load opening slice"),
			 D->LoadFromFile(FPaths::ProjectContentDir() / TEXT("Data/Narrative/opening_slice.json"), Error));
	TestEqual(TEXT("ten beats"), D->NumBeats(), 10);
	TestEqual(TEXT("opening"), D->CurrentBeatId(), FName("wake_pump"));
	TestFalse(TEXT("speaker exposed"), D->CurrentSpeaker().IsEmpty());
	TestFalse(TEXT("line exposed"), D->CurrentLine().IsEmpty());
	D->Tick(299);
	TestFalse(TEXT("not done before 300"), D->IsFinished());
	D->Tick(1);
	TestTrue(TEXT("300 seconds"), D->IsFinished());
	TestEqual(TEXT("finished id"), D->CurrentBeatId(), NAME_None);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrativeBeatStarts, "FortyDaysUnder.Narrative.BeatStartTimes", NarrativeTest::Flags)
bool FNarrativeBeatStarts::RunTest(const FString &Parameters)
{
	TStrongObjectPtr<USliceDirector> D(NewObject<USliceDirector>());
	TestEqual(TEXT("empty unknown"), D->BeatStartSeconds("one"), -1.0);
	TestEqual(TEXT("empty total"), D->TotalSeconds(), 0.0);
	FString Error;
	TestTrue(TEXT("load two"), D->LoadFromJsonString(NarrativeTest::TwoBeats, Error));
	TestEqual(TEXT("first at zero"), D->BeatStartSeconds("one"), 0.0);
	TestEqual(TEXT("second after first timeout"), D->BeatStartSeconds("two"), 2.0);
	TestEqual(TEXT("unknown beat"), D->BeatStartSeconds("three"), -1.0);
	TestEqual(TEXT("total"), D->TotalSeconds(), 5.0);
	D->NotifyEvent("Go");
	TestEqual(TEXT("starts are fixed, not elapsed"), D->BeatStartSeconds("two"), 2.0);
	TestTrue(TEXT("load opening slice"),
			 D->LoadFromFile(FPaths::ProjectContentDir() / TEXT("Data/Narrative/opening_slice.json"), Error));
	// These are the section starts in LS_OpeningSlice (seconds x 30 fps).
	const TPair<FName, double> Expected[] = {{"wake_pump", 0},		 {"read_manifold", 20}, {"check_watch", 55},
											 {"log_stock", 80},		 {"telemetry_fault", 110}, {"clear_fault", 140},
											 {"valve_stuck", 180},	 {"crank_valve", 205},	{"dome_look", 250},
											 {"log_close", 280}};
	for (const TPair<FName, double> &E : Expected)
	{
		TestEqual(*E.Key.ToString(), D->BeatStartSeconds(E.Key), E.Value);
	}
	TestEqual(TEXT("slice total"), D->TotalSeconds(), 300.0);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrativeSliceSync, "FortyDaysUnder.Narrative.SliceSync", NarrativeTest::Flags)
bool FNarrativeSliceSync::RunTest(const FString &Parameters)
{
	TestFalse(TEXT("timeout rollover stays put"), FSlicePlaybackSync::NeedsJump(20.03, 20.0, 0.25));
	TestFalse(TEXT("a frame early stays put"), FSlicePlaybackSync::NeedsJump(19.97, 20.0, 0.25));
	TestTrue(TEXT("early event jumps ahead"), FSlicePlaybackSync::NeedsJump(6.0, 20.0, 0.25));
	TestTrue(TEXT("sequence ahead jumps back"), FSlicePlaybackSync::NeedsJump(21.0, 20.0, 0.25));
	TestFalse(TEXT("first beat at rest"), FSlicePlaybackSync::NeedsJump(0.0, 0.0, 0.25));
	TestFalse(TEXT("unknown beat never jumps"), FSlicePlaybackSync::NeedsJump(6.0, -1.0, 0.25));
	TestFalse(TEXT("nan ignored"), FSlicePlaybackSync::NeedsJump(std::numeric_limits<double>::quiet_NaN(), 20.0, 0.25));
	TestTrue(TEXT("negative tolerance treated as zero"), FSlicePlaybackSync::NeedsJump(20.01, 20.0, -1.0));
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNarrativeDeath, "FortyDaysUnder.Narrative.DeathFromAnyState", NarrativeTest::Flags)
bool FNarrativeDeath::RunTest(const FString &Parameters)
{
	for (int32 Target = 0; Target <= static_cast<int32>(EAct2State::Dead); ++Target)
	{
		TStrongObjectPtr<UAct2StateMachine> M(NewObject<UAct2StateMachine>());
		const EAct2State State = static_cast<EAct2State>(Target);
		if (Target >= 1)
		{
			M->OnDayChanged(21);
		}
		if (Target >= 2)
		{
			M->OnDayChanged(40);
		}
		if (State >= EAct2State::A_Broadcasting && State <= EAct2State::A_EndingScuttled)
		{
			M->ChooseBroadcast();
			if (State >= EAct2State::A_FamilyDocked)
			{
				M->Advance("FamilyArrived");
			}
			if (State >= EAct2State::A_Siege48h)
			{
				M->Advance("HordeArrived");
			}
			if (State == EAct2State::A_EndingRescued)
			{
				M->TickSiege(48 * 3600);
			}
			if (State == EAct2State::A_EndingScuttled)
			{
				M->Advance("Scuttle");
			}
		}
		if (State >= EAct2State::B_Preparing && State <= EAct2State::B_EndingAlone)
		{
			M->ChooseGoAlone();
			if (State >= EAct2State::B_Swim)
			{
				NarrativeTest::Prepare(*M);
			}
			if (State >= EAct2State::B_Siphon)
			{
				M->Advance("ReachedMarina");
			}
			if (State >= EAct2State::B_Return)
			{
				M->Advance("FuelSiphoned");
			}
			if (State == EAct2State::B_EndingAlone)
			{
				M->Advance("Repressurised");
			}
		}
		if (State == EAct2State::Dead)
		{
			M->NotifyDeath();
		}
		TestEqual(TEXT("reached target"), M->GetState(), State);
		M->NotifyDeath();
		M->OnDayChanged(50);
		M->TickSiege(1e300);
		M->ChooseBroadcast();
		M->ChooseGoAlone();
		M->Advance("Repressurised");
		M->CompletePrepTask(EPrepTask::EmergencyDeco);
		TestEqual(TEXT("death is absorbing"), M->GetState(), EAct2State::Dead);
	}
	return true;
}
#endif
