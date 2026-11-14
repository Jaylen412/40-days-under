#include "Input/DualSenseFeedbackComponent.h"
#include "Input/DualSenseDeviceRouting.h"
#include "GenericPlatform/InputDeviceRegistry.h"
#include "Engine/GameInstance.h"
#include "GameMapsSettings.h"
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
	FDualSenseDeviceRoute ResolveDevice(const APlayerController* Controller)
	{
		const ULocalPlayer* LocalPlayer = Controller ? Controller->GetLocalPlayer() : nullptr;
		if (!LocalPlayer || !FSlateApplication::IsInitialized() || !FSlateApplication::Get().GetInputInterface())
		{
			return {};
		}
		IPlatformInputDeviceMapper& Mapper = IPlatformInputDeviceMapper::Get();
		const FPlatformUserId UserId = LocalPlayer->GetPlatformUserId();
		TArray<FInputDeviceId> DeviceIds;
		Mapper.GetAllInputDevicesForUser(UserId, DeviceIds);
		TArray<FDualSenseDeviceCandidate> Candidates;
		for (FInputDeviceId DeviceId : DeviceIds)
		{
			const TOptional<FInputDeviceDescriptor> Descriptor = FInputDeviceRegistry::FindDescriptor(DeviceId);
			if (Descriptor.IsSet())
			{
				Candidates.Add({DeviceId, Descriptor->InputDeviceName,
					Mapper.GetInputDeviceConnectionState(DeviceId) == EInputDeviceConnectionState::Connected});
			}
		}
		return ResolveDualSenseDevice(Candidates, [&Mapper, UserId](FInputDeviceId DeviceId, int32& ControllerId)
		{
			return Mapper.RemapUserAndDeviceToControllerId(UserId, ControllerId, DeviceId);
		});
	}

	// Same raw property path as UInputDeviceProperty; retain the validated device's route.
	void SendDeviceProperty(const FDualSenseDeviceRoute& Route, const FInputDeviceProperty& Property)
	{
		if (Route.IsValid() && FSlateApplication::IsInitialized())
		{
			if (IInputInterface* InputInterface = FSlateApplication::Get().GetInputInterface())
			{
				InputInterface->SetDeviceProperty(Route.ControllerId, &Property);
			}
		}
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
	return ResolveDevice(World->GetFirstPlayerController()).IsValid();
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
	// Leave the pad as we found it: no stuck resistance, vibration or lightbar colour.
	if (HasSupportedDevice())
	{
		const FDualSenseDeviceRoute Route = ResolveDevice(GetWorld()->GetFirstPlayerController());
		SendDeviceProperty(Route, FInputDeviceTriggerResetProperty());
		SendDeviceProperty(Route, FInputDeviceLightColorProperty(false, FColor::Black));
	}
	bLightbarSent = false;
	StopPumpHaptic();

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
		StopPumpHaptic();
		bLightbarSent = false;
		return;
	}
	ApplyLightbar(FPlatformTime::Seconds());
	UpdatePumpHaptic(DeltaTime);
}

void UDualSenseFeedbackComponent::ApplyTrigger()
{
	const UWorld* World = GetWorld();
	const APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	const FDualSenseDeviceRoute Route = ResolveDevice(Controller);
	if (!Route.IsValid())
	{
		return;
	}

	const FTriggerFeedback Design = FDualSenseFeedbackProfile::TriggerFor(Action, Progress01, Snapshot);
	const FTriggerFeedback Feedback = FDualSenseFeedbackProfile::ToDeviceUnits(Design);

	// A trigger holds one mode: each property replaces the last, so send exactly one.
	switch (FDualSenseFeedbackProfile::SendKindFor(Design))
	{
	case ETriggerSendKind::Vibration:
	{
		FInputDeviceTriggerVibrationProperty Vibration;
		Vibration.AffectedTriggers = EInputDeviceTriggerMask::Right;
		Vibration.TriggerPosition = Feedback.StartPosition;
		Vibration.VibrationFrequency = Feedback.VibrationFrequencyHz;
		Vibration.VibrationAmplitude = Feedback.VibrationAmplitude;
		SendDeviceProperty(Route, Vibration);
		break;
	}
	case ETriggerSendKind::Resistance:
	{
		FInputDeviceTriggerResistanceProperty Resistance;
		Resistance.AffectedTriggers = EInputDeviceTriggerMask::Right;
		Resistance.StartPosition = Feedback.StartPosition;
		Resistance.EndPosition = Feedback.EndPosition;
		Resistance.StartStrengh = Feedback.StartStrength;
		Resistance.EndStrengh = Feedback.EndStrength;
		SendDeviceProperty(Route, Resistance);
		break;
	}
	case ETriggerSendKind::Reset:
	default:
		SendDeviceProperty(Route, FInputDeviceTriggerResetProperty());
		break;
	}
}

void UDualSenseFeedbackComponent::ApplyLightbar(double RealTimeSeconds)
{
	const UWorld* World = GetWorld();
	const APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	const FDualSenseDeviceRoute Route = ResolveDevice(Controller);
	if (!Route.IsValid())
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
	SendDeviceProperty(Route, FInputDeviceLightColorProperty(true, Color));
}

void UDualSenseFeedbackComponent::UpdatePumpHaptic(float DeltaTime)
{
	const FPumpHaptic Haptic = FDualSenseFeedbackProfile::PumpHaptic(Snapshot);
	if (Haptic.Amplitude <= 0.0f || Haptic.PulseHz <= 0.0f)
	{
		StopPumpHaptic();
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
	const FDualSenseDeviceRoute Route = ResolveDevice(Controller);
	// PlayerController haptics use a legacy index, with the viewport multiplayer offset.
	int32 HapticIndex = Controller ? Controller->GetInputIndex() : INDEX_NONE;
	if (World && World->GetGameInstance() && World->GetGameInstance()->GetNumLocalPlayers() > 1
		&& GetDefault<UGameMapsSettings>()->bOffsetPlayerGamepadIds)
	{
		--HapticIndex;
	}
	if (Route.IsValid() && Route.ControllerId == HapticIndex && PumpThumpEffect)
	{
		Controller->PlayHapticEffect(PumpThumpEffect, EControllerHand::Left, Haptic.Amplitude, false);
		PumpHapticController = Controller;
	}
	else
	{
		StopPumpHaptic();
	}
}

void UDualSenseFeedbackComponent::StopPumpHaptic()
{
	if (APlayerController* Controller = PumpHapticController.Get())
	{
		Controller->StopHapticEffect(EControllerHand::Left);
	}
	PumpHapticController.Reset();
	PumpPhase = 0.0f;
}
