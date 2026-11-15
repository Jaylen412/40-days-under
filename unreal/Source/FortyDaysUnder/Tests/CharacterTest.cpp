#include "Misc/AutomationTest.h"
#include "Components/CapsuleComponent.h"
#include "Character/UnderwaterCharacter.h"
#include "Character/UnderwaterMovementComponent.h"
#include "Simulation/VitalsSnapshot.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace FortyDaysUnderCharacterTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterDimensionsTest, "FortyDaysUnder.Character.Dimensions", FortyDaysUnderCharacterTest::Flags)
bool FCharacterDimensionsTest::RunTest(const FString& Parameters)
{
	const AUnderwaterCharacter* CDO = GetDefault<AUnderwaterCharacter>();
	TestEqual(TEXT("Capsule half-height"), CDO->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), 87.63f, 0.001f);
	TestEqual(TEXT("Capsule radius"), CDO->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), 34.0f, 0.001f);
	TestTrue(TEXT("Movement component class"), CDO->GetCharacterMovement()->IsA(UUnderwaterMovementComponent::StaticClass()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterSwimSpeedTest, "FortyDaysUnder.Character.SwimSpeed", FortyDaysUnderCharacterTest::Flags)
bool FCharacterSwimSpeedTest::RunTest(const FString& Parameters)
{
	const UUnderwaterMovementComponent* Move = GetDefault<UUnderwaterMovementComponent>();
	TestEqual(TEXT("Full stamina"), Move->GetEffectiveSwimSpeed(100.0f, EAirStage::Nominal), 180.0f, 0.01f);
	TestEqual(TEXT("Zero stamina"), Move->GetEffectiveSwimSpeed(0.0f, EAirStage::Nominal), 72.0f, 0.01f);
	TestEqual(TEXT("Hypoxic"), Move->GetEffectiveSwimSpeed(100.0f, EAirStage::Hypoxic), 90.0f, 0.01f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterDcsEffectsTest, "FortyDaysUnder.Character.DcsEffects", FortyDaysUnderCharacterTest::Flags)
bool FCharacterDcsEffectsTest::RunTest(const FString& Parameters)
{
	FVitalsSnapshot S;
	S.DcsSeverity = 1.0;
	TestEqual(TEXT("Latency at 1"), AUnderwaterCharacter::LatencyFor(S), 0.25f, 0.0001f);
	TestEqual(TEXT("Low-pass at 1"), AUnderwaterCharacter::LowPassHzFor(S), 800.0f, 0.01f);
	S.DcsSeverity = 0.0;
	TestEqual(TEXT("Latency at 0"), AUnderwaterCharacter::LatencyFor(S), 0.0f, 0.0001f);
	TestEqual(TEXT("Low-pass at 0"), AUnderwaterCharacter::LowPassHzFor(S), 20000.0f, 0.01f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterVignetteTest, "FortyDaysUnder.Character.Vignette", FortyDaysUnderCharacterTest::Flags)
bool FCharacterVignetteTest::RunTest(const FString& Parameters)
{
	FVitalsSnapshot S;
	const EAirStage Stages[] = { EAirStage::Nominal, EAirStage::Impaired, EAirStage::Hypoxic, EAirStage::Critical };
	const float Expected[] = { 0.0f, 0.3f, 0.6f, 1.0f };
	for (int32 i = 0; i < 4; ++i)
	{
		S.AirStage = Stages[i];
		TestEqual(*FString::Printf(TEXT("Vignette stage %d"), i), AUnderwaterCharacter::VignetteFor(S), Expected[i], 0.0001f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterForceSubmergedTest, "FortyDaysUnder.Character.ForceSubmerged", FortyDaysUnderCharacterTest::Flags)
bool FCharacterForceSubmergedTest::RunTest(const FString& Parameters)
{
	UUnderwaterMovementComponent* Move = NewObject<UUnderwaterMovementComponent>(GetTransientPackage());
	Move->bForceSubmerged = true;
	TestTrue(TEXT("Forced: in water with no volume"), Move->IsInWater());
	TestEqual(TEXT("Forced: full immersion"), Move->ImmersionDepth(), 1.0f, 0.0001f);
	Move->bForceSubmerged = false;
	TestFalse(TEXT("Not forced: not in water without a volume"), Move->IsInWater());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterLabToggleTest, "FortyDaysUnder.Character.LabToggle", FortyDaysUnderCharacterTest::Flags)
bool FCharacterLabToggleTest::RunTest(const FString& Parameters)
{
	AUnderwaterCharacter* Char = NewObject<AUnderwaterCharacter>(GetTransientPackage());
	UUnderwaterMovementComponent* Move = Cast<UUnderwaterMovementComponent>(Char->GetCharacterMovement());
	if (!TestNotNull(TEXT("Movement component"), Move))
	{
		return false;
	}
	Char->SetInsidePressurisedLab(false);
	TestTrue(TEXT("Outside lab: forced submerged"), Move->bForceSubmerged);
	TestFalse(TEXT("Outside lab: flag"), Char->bInsidePressurisedLab);
	Char->SetInsidePressurisedLab(true);
	TestFalse(TEXT("Inside lab: not forced"), Move->bForceSubmerged);
	TestTrue(TEXT("Inside lab: flag"), Char->bInsidePressurisedLab);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterLookPitchSignTest, "FortyDaysUnder.Character.LookPitchSign", FortyDaysUnderCharacterTest::Flags)
bool FCharacterLookPitchSignTest::RunTest(const FString& Parameters)
{
	// Look input arrives as "up is positive"; the final pitch change must be positive (look up) in every config.
	for (const bool bLegacy : {false, true})
	{
		for (const float Scale : {-2.5f, 1.0f, 2.5f})
		{
			const float Applied = bLegacy ? Scale : 1.0f;
			const float PitchDelta = AUnderwaterCharacter::PitchSignFor(bLegacy, Scale) * 1.0f * Applied;
			TestTrue(FString::Printf(TEXT("up looks up (legacy=%d scale=%.1f)"), bLegacy, Scale), PitchDelta > 0.0f);
		}
	}
	return true;
}
#endif
