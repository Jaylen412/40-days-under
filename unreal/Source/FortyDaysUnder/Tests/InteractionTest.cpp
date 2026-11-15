#include "Interaction/SliceInteractionModel.h"
#include "Misc/AutomationTest.h"
#include <limits>

#if WITH_DEV_AUTOMATION_TESTS
namespace InteractionTest
{
constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
using K = ESliceInteractionKind;
} // namespace InteractionTest

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInteractionUse, "FortyDaysUnder.Interaction.Use", InteractionTest::Flags)
bool FInteractionUse::RunTest(const FString& Parameters)
{
	using namespace InteractionTest;
	FSliceInteractionModel M;
	TestFalse(TEXT("focus alone does nothing"), M.Update(K::Use, true, false, false, 1.0, 0.0));
	TestFalse(TEXT("holding without a press does nothing"), M.Update(K::Use, true, true, false, 10.0, 0.0));
	TestFalse(TEXT("press while looking away does nothing"), M.Update(K::Use, false, true, true, 0.1, 0.0));
	TestTrue(TEXT("press while focused fires"), M.Update(K::Use, true, true, true, 0.0, 0.0));
	TestFalse(TEXT("fires once"), M.Update(K::Use, true, true, true, 0.1, 0.0));
	TestEqual(TEXT("fired reads full"), M.Progress01(0.0), 1.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInteractionGaze, "FortyDaysUnder.Interaction.Gaze", InteractionTest::Flags)
bool FInteractionGaze::RunTest(const FString& Parameters)
{
	using namespace InteractionTest;
	FSliceInteractionModel M;
	TestFalse(TEXT("one second of two"), M.Update(K::Gaze, true, false, false, 1.0, 2.0));
	TestEqual(TEXT("half way"), M.Progress01(2.0), 0.5);
	TestFalse(TEXT("looking away resets"), M.Update(K::Gaze, false, false, false, 0.1, 2.0));
	TestEqual(TEXT("reset"), M.Progress01(2.0), 0.0);
	TestFalse(TEXT("start again"), M.Update(K::Gaze, true, false, false, 1.9, 2.0));
	TestFalse(TEXT("negative time ignored"), M.Update(K::Gaze, true, false, false, -5.0, 2.0));
	TestFalse(TEXT("nan time ignored"),
			  M.Update(K::Gaze, true, false, false, std::numeric_limits<double>::quiet_NaN(), 2.0));
	TestTrue(TEXT("dwell reached, no input needed"), M.Update(K::Gaze, true, false, false, 0.1, 2.0));
	TestFalse(TEXT("fires once"), M.Update(K::Gaze, true, false, false, 5.0, 2.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInteractionHold, "FortyDaysUnder.Interaction.Hold", InteractionTest::Flags)
bool FInteractionHold::RunTest(const FString& Parameters)
{
	using namespace InteractionTest;
	FSliceInteractionModel M;
	TestFalse(TEXT("focused but not held"), M.Update(K::Hold, true, false, false, 3.0, 5.0));
	TestEqual(TEXT("no progress without hold"), M.Progress01(5.0), 0.0);
	TestFalse(TEXT("two of five"), M.Update(K::Hold, true, true, true, 2.0, 5.0));
	TestFalse(TEXT("letting go pauses"), M.Update(K::Hold, true, false, false, 4.0, 5.0));
	TestEqual(TEXT("effort kept"), M.Progress01(5.0), 0.4);
	TestFalse(TEXT("four of five"), M.Update(K::Hold, true, true, true, 2.0, 5.0));
	TestFalse(TEXT("looking away abandons"), M.Update(K::Hold, false, true, false, 0.1, 5.0));
	TestEqual(TEXT("abandoned"), M.Progress01(5.0), 0.0);
	TestFalse(TEXT("almost"), M.Update(K::Hold, true, true, false, 4.9, 5.0));
	TestTrue(TEXT("done"), M.Update(K::Hold, true, true, false, 0.2, 5.0));
	TestFalse(TEXT("fires once"), M.Update(K::Hold, true, true, false, 9.0, 5.0));
	return true;
}
#endif
