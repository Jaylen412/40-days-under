#include "Camera/CameraComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Character/ViewmodelArm.h"
#include "Character/WatchDisplay.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Misc/AutomationTest.h"
#include "Simulation/BuhlmannModel.h"
#include <limits>

#if WITH_DEV_AUTOMATION_TESTS
namespace ViewmodelTest
{
constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWatchDisplayLines, "FortyDaysUnder.Viewmodel.WatchLines", ViewmodelTest::Flags)
bool FWatchDisplayLines::RunTest(const FString& Parameters)
{
	FVitalsSnapshot S;
	S.AmbientBar = FBuhlmannModel::SurfacePressureBar + FBuhlmannModel::BarPerMetreFresh * 15.0;
	S.CeilingBar = FBuhlmannModel::SurfacePressureBar + FBuhlmannModel::BarPerMetreFresh * 7.26;
	S.Day = 1;
	S.GameSeconds = 6 * 3600 + 42 * 60 + 59;
	TArray<FString> L = FWatchDisplay::Lines(S);
	TestEqual(TEXT("three lines"), L.Num(), FWatchDisplay::NumLines);
	TestEqual(TEXT("depth"), L[0], FString(TEXT("15.0 m")));
	TestEqual(TEXT("ceiling"), L[1], FString(TEXT("CEIL 7.3 m")));
	TestEqual(TEXT("clock"), L[2], FString(TEXT("D1 06:42")));

	S.CeilingBar = 1.0;   // below surface pressure: no ceiling
	TestEqual(TEXT("no deco"), FWatchDisplay::Lines(S)[1], FString(TEXT("NO DECO")));
	S.bInDecoCycle = true;
	S.DecoHoursRemaining = 16.96;
	TestEqual(TEXT("deco cycle"), FWatchDisplay::Lines(S)[1], FString(TEXT("DECO 17.0 h")));

	S.Day = 3;
	S.GameSeconds = 2 * 86400 + 23 * 3600 + 59 * 60;
	TestEqual(TEXT("day wraps clock"), FWatchDisplay::Lines(S)[2], FString(TEXT("D3 23:59")));
	TestEqual(TEXT("surface depth"), FWatchDisplay::DepthMetres(FBuhlmannModel::SurfacePressureBar), 0.0);
	TestEqual(TEXT("never negative"), FWatchDisplay::DepthMetres(0.5), 0.0);
	TestEqual(TEXT("NaN safe"), FWatchDisplay::DepthMetres(std::numeric_limits<double>::quiet_NaN()), 0.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FViewmodelArmMotion, "FortyDaysUnder.Viewmodel.ArmMotion", ViewmodelTest::Flags)
bool FViewmodelArmMotion::RunTest(const FString& Parameters)
{
	FViewmodelArm A;
	A.RaiseSeconds = 0.3;
	TestFalse(TEXT("hidden at rest"), A.IsVisible());
	A.Update(0.15, true);
	TestEqual(TEXT("half way"), A.Alpha, 0.5, 1e-9);
	TestEqual(TEXT("eased midpoint"), A.Eased(), 0.5, 1e-9);
	A.Update(1.0, true);
	TestEqual(TEXT("clamped at 1"), A.Alpha, 1.0);
	A.Update(-1.0, false);
	A.Update(std::numeric_limits<double>::quiet_NaN(), false);
	TestEqual(TEXT("bad deltas ignored"), A.Alpha, 1.0);
	A.Update(0.3, false);
	TestEqual(TEXT("back to rest"), A.Alpha, 0.0, 1e-9);
	TestFalse(TEXT("hidden again"), A.IsVisible());

	const FTransform Rest(FQuat::Identity, FVector(0, 0, -45));
	const FTransform Up(FQuat(FVector::UpVector, PI / 2), FVector(30, 0, -10));
	TestTrue(TEXT("blend 0 = rest"), FViewmodelArm::Blend(Rest, Up, 0.0).Equals(Rest));
	TestTrue(TEXT("blend 1 = raised"), FViewmodelArm::Blend(Rest, Up, 1.0).Equals(Up));
	TestTrue(TEXT("blend clamps"), FViewmodelArm::Blend(Rest, Up, 7.0).Equals(Up));

	const FTransform P = FViewmodelArm::Pose(FVector(1, 2, 3), FVector(0, 1, 0), FVector(0, 0, 1));
	TestTrue(TEXT("pose forearm axis"), P.GetUnitAxis(EAxis::X).Equals(FVector(0, 1, 0), 1e-6));
	TestTrue(TEXT("pose back of wrist"), P.GetUnitAxis(EAxis::Z).Equals(FVector(0, 0, 1), 1e-6));
	return true;
}

/** The character owns both arms and the watch, the watch rides the left arm, and the raised pose shows the watch face. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FViewmodelCharacter, "FortyDaysUnder.Viewmodel.Character", ViewmodelTest::Flags)
bool FViewmodelCharacter::RunTest(const FString& Parameters)
{
	AUnderwaterCharacter* C = NewObject<AUnderwaterCharacter>(GetTransientPackage());
	if (!TestNotNull(TEXT("arm left"), C->ArmLeft.Get()) || !TestNotNull(TEXT("arm right"), C->ArmRight.Get()) ||
		!TestNotNull(TEXT("watch"), C->Watch.Get()))
	{
		return false;
	}
	TestTrue(TEXT("watch on left arm"), C->Watch->GetAttachParent() == C->ArmLeft);
	TestTrue(TEXT("arms on camera"), C->ArmLeft->GetAttachParent() == C->FirstPersonCamera.Get());
	TestTrue(TEXT("owner only"), C->ArmLeft->bOnlyOwnerSee && C->Watch->bOnlyOwnerSee);
	TestEqual(TEXT("watch lines"), C->WatchLines.Num(), FWatchDisplay::NumLines);
	TestNotNull(TEXT("arm mesh asset"), C->ArmLeft->GetStaticMesh().Get());
	TestNotNull(TEXT("watch mesh asset"), C->Watch->GetStaticMesh().Get());

	// Raised pose puts the watch face in front of the camera and turned toward it.
	const FTransform WatchRaised = C->Watch->GetRelativeTransform() * C->ArmLeftRaised;
	TestTrue(TEXT("watch ahead of eye"), WatchRaised.GetLocation().X > 15.0);
	TestTrue(TEXT("watch face toward eye"), WatchRaised.GetUnitAxis(EAxis::Z).X < -0.5);
	TestTrue(TEXT("rest is below raised"), C->ArmLeftRest.GetLocation().Z < C->ArmLeftRaised.GetLocation().Z - 30.0);

	C->RefreshWatch();
	TestTrue(TEXT("watch shows depth"), C->WatchLines[0]->Text.ToString().EndsWith(TEXT(" m")));
	return true;
}
#endif
