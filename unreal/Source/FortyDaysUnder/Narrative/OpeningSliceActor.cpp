#include "Narrative/OpeningSliceActor.h"
#include "Narrative/IntroFootageComponent.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Misc/Paths.h"
#include "Narrative/SliceDirector.h"
#include "Narrative/SlicePlaybackSync.h"

DEFINE_LOG_CATEGORY_STATIC(LogOpeningSlice, Log, All);

AOpeningSliceActor::AOpeningSliceActor()
{
	PrimaryActorTick.bCanEverTick = true;
	IntroFootage = CreateDefaultSubobject<UIntroFootageComponent>(TEXT("IntroFootage"));
}

void AOpeningSliceActor::BeginPlay()
{
	Super::BeginPlay();
	if (IntroFootage)
	{
		IntroFootage->OnFinished.AddDynamic(this, &AOpeningSliceActor::StartSlice);
		if (IntroFootage->StartIntro())
		{
			return;
		}
		IntroFootage->OnFinished.RemoveDynamic(this, &AOpeningSliceActor::StartSlice);
	}
	StartSlice();
}

void AOpeningSliceActor::StartSlice()
{
	if (Director)
	{
		return;
	}
	Director = NewObject<USliceDirector>(this);
	// Bind before loading: a successful load announces the first beat.
	Director->OnBeatStarted.AddDynamic(this, &AOpeningSliceActor::HandleBeatStarted);
	FString Error;
	if (!Director->LoadFromFile(FPaths::ProjectContentDir() / SliceFile, Error))
	{
		UE_LOG(LogOpeningSlice, Error, TEXT("Opening slice not started: %s"), *Error);
	}
	else
	{
		UE_LOG(LogOpeningSlice, Log, TEXT("Opening slice started: %d beats, first %s"), Director->NumBeats(), *Director->CurrentBeatId().ToString());
	}
}

void AOpeningSliceActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Director)
	{
		Director->Tick(DeltaSeconds);
	}
}

bool AOpeningSliceActor::NotifyEvent(FName Event)
{
	return Director && Director->NotifyEvent(Event);
}

double AOpeningSliceActor::GetSequenceSeconds() const
{
	const ULevelSequencePlayer* Player = SequenceActor ? SequenceActor->GetSequencePlayer() : nullptr;
	return Player ? Player->GetCurrentTime().AsSeconds() : -1.0;
}

void AOpeningSliceActor::HandleBeatStarted(FName BeatId)
{
	ULevelSequencePlayer* Player = SequenceActor ? SequenceActor->GetSequencePlayer() : nullptr;
	if (!Player || !Director)
	{
		return;
	}
	const double Start = Director->BeatStartSeconds(BeatId);
	if (FSlicePlaybackSync::NeedsJump(Player->GetCurrentTime().AsSeconds(), Start, ResyncToleranceSeconds))
	{
		Player->SetPlaybackPosition(
			FMovieSceneSequencePlaybackParams(static_cast<float>(Start), EUpdatePositionMethod::Jump));
	}
	if (!Player->IsPlaying())
	{
		Player->Play();
	}
	UE_LOG(LogOpeningSlice, Log, TEXT("Beat %s started at %.2f s"), *BeatId.ToString(), Start);
}
