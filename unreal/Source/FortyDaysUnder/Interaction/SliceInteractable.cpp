#include "Interaction/SliceInteractable.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "EngineUtils.h"
#include "Narrative/OpeningSliceActor.h"
#include "Narrative/SliceDirector.h"

namespace
{
	constexpr float BeaconIdleLumens = 250.0f;
	constexpr float BeaconFullLumens = 1600.0f;
}

ASliceInteractable::ASliceInteractable()
{
	PrimaryActorTick.bCanEverTick = true;

	TraceVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TraceVolume"));
	SetRootComponent(TraceVolume);
	TraceVolume->SetBoxExtent(FVector(20.0f));
	TraceVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TraceVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TraceVolume->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	TraceVolume->SetCanEverAffectNavigation(false);

	Beacon = CreateDefaultSubobject<UPointLightComponent>(TEXT("Beacon"));
	Beacon->SetupAttachment(TraceVolume);
	Beacon->IntensityUnits = ELightUnits::Lumens;
	Beacon->Intensity = BeaconIdleLumens;
	Beacon->AttenuationRadius = 300.0f;
	Beacon->LightColor = FColor(60, 230, 255);
	Beacon->CastShadows = false;
	Beacon->SetVisibility(false);
}

void ASliceInteractable::BeginPlay()
{
	Super::BeginPlay();
	if (!Slice)
	{
		const TActorIterator<AOpeningSliceActor> It(GetWorld());
		Slice = It ? *It : nullptr;
	}
	if (SpinActor)
	{
		SpinBase = SpinActor->GetActorQuat();
	}
}

bool ASliceInteractable::IsLive() const
{
	const USliceDirector* Director = Slice ? Slice->GetDirector() : nullptr;
	return Director && !Model.bFired && !Beat.IsNone() && Director->CurrentBeatId() == Beat;
}

void ASliceInteractable::Advance(double DeltaSeconds, bool bFocused, bool bHeld, bool bPressed)
{
	if (!IsLive())
	{
		return;
	}
	if (Model.Update(Kind, bFocused, bHeld, bPressed, DeltaSeconds, RequiredSeconds) && Slice)
	{
		Slice->NotifyEvent(Event);
	}
}

void ASliceInteractable::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const bool bLive = IsLive();
	if (bLive && !bWasLive)
	{
		OnBecameLive();
	}
	else if (!bLive && bWasLive)
	{
		OnCompleted();
	}
	bWasLive = bLive;
	if (bLive)
	{
		const double Alpha = GetProgress01();
		Beacon->SetIntensity(FMath::Lerp(BeaconIdleLumens, BeaconFullLumens, static_cast<float>(Alpha)));
		ApplySpin(Alpha);
	}
}

void ASliceInteractable::OnBecameLive()
{
	if (bRequiresFocus)
	{
		TraceVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Beacon->SetVisibility(true);
	}
	SetShown(ShowOnActivate, true);
	SetShown(HideOnActivate, false);
}

void ASliceInteractable::OnCompleted()
{
	if (bCompleted)
	{
		return;
	}
	bCompleted = true;
	TraceVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Beacon->SetVisibility(false);
	SetShown(ShowOnComplete, true);
	SetShown(HideOnComplete, false);
	// A beat that timed out still leaves the world as if the task was done.
	ApplySpin(1.0);
}

void ASliceInteractable::ApplySpin(double Alpha)
{
	if (SpinActor && !FMath::IsNearlyZero(SpinDegrees))
	{
		const FQuat Turn(FVector::XAxisVector, FMath::DegreesToRadians(SpinDegrees * Alpha));
		SpinActor->SetActorRotation(SpinBase * Turn);
	}
}

void ASliceInteractable::SetShown(const TArray<TObjectPtr<AActor>>& Actors, bool bShown)
{
	for (AActor* Actor : Actors)
	{
		if (Actor)
		{
			Actor->SetActorHiddenInGame(!bShown);
		}
	}
}
