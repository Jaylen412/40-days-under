#include "Input/DualSenseFeedbackComponent.h"
#include "Survival/SurvivalVitalsComponent.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "GenericPlatform/IInputInterface.h"
#include "Framework/Application/SlateApplication.h"
#include "Haptics/HapticFeedbackEffect_Base.h"

namespace
{
	/**
	 * Sends a raw device property to the pad. This is the same path UInputDeviceProperty uses
	 * internally; the UObject property classes keep their data protected or in curve assets,
	 * which does not suit values computed every frame.
	 */
	void SendDeviceProperty(const APlayerController* Controller, const FInputDeviceProperty& Property)
	{
		const ULocalPlayer* LocalPlayer = Controller ? Controller->GetLocalPlayer() : nullptr;
		if (!LocalPlayer || !FSlateApplication::IsInitialized())
		{
			return;
		}
		IInputInterface* InputInterface = FSlateApplication::Get().GetInputInterface();
		if (!InputInterface)
		{
			return;
		}
		int32 ControllerId = INDEX_NONE;
		IPlatformInputDeviceMapper::Get().RemapUserAndDeviceToControllerId(LocalPlayer->GetPlatformUserId(), ControllerId);
		InputInterface->SetDeviceProperty(ControllerId, &Property);
	}
}

UDualSenseFeedbackComponent::UDualSenseFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

bool UDualSenseFeedbackComponent::HasSupportedDevice() const
{
	const AActor* Owner = GetOwner();
	const UWorld* World = GetWorld();
	if (!Owner || !World || !UInputDeviceSubsystem::Get())
	{
		return false;
	}
	return World->GetFirstPlayerController() != nullptr && World->GetFirstPlayerController()->IsLocalPlayerController();
}

void UDualSenseFeedbackComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* Owner = GetOwner())
	{
		if (USurvivalVitalsComponent* Vitals = Owner->FindComponentByClass<USurvivalVitalsComponent>())
		{
			BoundVitals = Vitals;
			Vitals->OnVitalsChanged.AddDynamic(this, &UDualSenseFeedbackComponent::HandleVitalsChanged);
			Snapshot = Vitals->GetSnapshot();
		}
	}
}

void UDualSenseFeedbackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (USurvivalVitalsComponent* Vitals = BoundVitals.Get())
	{
		Vitals->OnVitalsChanged.RemoveDynamic(this, &UDualSenseFeedbackComponent::HandleVitalsChanged);
	}
	BoundVitals.Reset();

	Super::EndPlay(EndPlayReason);
}

void UDualSenseFeedbackComponent::SetTriggerAction(ETriggerAction NewAction, float NewProgress01)
{
	Action = NewAction;
	Progress01 = FMath::Clamp(NewProgress01, 0.0f, 1.0f);
	if (!HasSupportedDevice())
	{
		return;
	}
	ApplyTrigger();
}

void UDualSenseFeedbackComponent::SetHordeProximity(float NewProximity01)
{
	HordeProximity01 = FMath::Clamp(NewProximity01, 0.0f, 1.0f);
	if (!HasSupportedDevice())
	{
		return;
	}
	ApplyLightbar(FPlatformTime::Seconds());
}

void UDualSenseFeedbackComponent::HandleVitalsChanged(const FVitalsSnapshot& NewSnapshot)
{
	Snapshot = NewSnapshot;
	if (!HasSupportedDevice())
	{
		return;
	}
	ApplyTrigger();
	ApplyLightbar(FPlatformTime::Seconds());
}

void UDualSenseFeedbackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!HasSupportedDevice())
	{
		return;
	}
	ApplyLightbar(FPlatformTime::Seconds());
	UpdatePumpHaptic(DeltaTime);
}

void UDualSenseFeedbackComponent::ApplyTrigger()
{
	const UWorld* World = GetWorld();
	const APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	if (!Controller)
	{
		return;
	}

	const FTriggerFeedback Feedback = FDualSenseFeedbackProfile::TriggerFor(Action, Progress01, Snapshot);

	FInputDeviceTriggerResistanceProperty Resistance;
	Resistance.AffectedTriggers = EInputDeviceTriggerMask::Right;
	Resistance.StartPosition = Feedback.StartPosition;
	Resistance.EndPosition = Feedback.EndPosition;
	Resistance.StartStrengh = Feedback.StartStrength;
	Resistance.EndStrengh = Feedback.EndStrength;
	SendDeviceProperty(Controller, Resistance);

	FInputDeviceTriggerVibrationProperty Vibration;
	Vibration.AffectedTriggers = EInputDeviceTriggerMask::Right;
	Vibration.TriggerPosition = Feedback.bVibrate ? Feedback.StartPosition : 0;
	Vibration.VibrationFrequency = Feedback.bVibrate ? Feedback.VibrationFrequencyHz : 0;
	Vibration.VibrationAmplitude = Feedback.bVibrate ? Feedback.VibrationAmplitude : 0;
	SendDeviceProperty(Controller, Vibration);
}

void UDualSenseFeedbackComponent::ApplyLightbar(double RealTimeSeconds)
{
	const UWorld* World = GetWorld();
	const APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	if (!Controller)
	{
		return;
	}

	const FColor Color = FDualSenseFeedbackProfile::LightbarColor(Snapshot, HordeProximity01, RealTimeSeconds).ToFColorSRGB();
	if (bLightbarSent && Color == LastLightbar)
	{
		return;
	}
	bLightbarSent = true;
	LastLightbar = Color;
	SendDeviceProperty(Controller, FInputDeviceLightColorProperty(true, Color));
}

void UDualSenseFeedbackComponent::UpdatePumpHaptic(float DeltaTime)
{
	const FPumpHaptic Haptic = FDualSenseFeedbackProfile::PumpHaptic(Snapshot);
	if (Haptic.Amplitude <= 0.0f || Haptic.PulseHz <= 0.0f)
	{
		PumpPhase = 0.0f;
		return;
	}

	// Jitter randomly stretches or shrinks each beat so a failing pump stutters.
	const float Rate = Haptic.PulseHz * (1.0f + Haptic.Jitter * FMath::FRandRange(-0.5f, 0.5f));
	PumpPhase += DeltaTime * Rate;
	if (PumpPhase < 1.0f)
	{
		return;
	}
	PumpPhase = FMath::Fmod(PumpPhase, 1.0f);

	const UWorld* World = GetWorld();
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	if (Controller && PumpThumpEffect)
	{
		Controller->PlayHapticEffect(PumpThumpEffect, EControllerHand::Left, Haptic.Amplitude, false);
	}
}
