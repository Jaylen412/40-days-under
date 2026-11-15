#include "Narrative/IntroFootageComponent.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "FileMediaSource.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaTexture.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScaleBox.h"

DEFINE_LOG_CATEGORY_STATIC(LogIntroFootage, Log, All);

UIntroFootageComponent::UIntroFootageComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bTickEvenWhenPaused = true;
}

bool UIntroFootageComponent::StartIntro()
{
	UWorld* World = GetWorld();
	UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
	if (!bEnabled || bPlaying || !Viewport || IsRunningDedicatedServer() || FApp::IsUnattended())
	{
		return false;
	}

	const FString ContentDir = FPaths::ProjectContentDir();
	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *(ContentDir / ManifestFile)))
	{
		UE_LOG(LogIntroFootage, Warning, TEXT("No footage manifest at %s"), *ManifestFile);
		return false;
	}
	TArray<FFootageClip> Clips;
	FString Error;
	if (!FFootagePlaylist::ParseJson(Json, Clips, Error))
	{
		UE_LOG(LogIntroFootage, Error, TEXT("Footage manifest rejected: %s"), *Error);
		return false;
	}
	const int32 Listed = Clips.Num();
	Playlist.Reset(FFootagePlaylist::KeepExisting(
		Clips, ContentDir, [](const FString& Path) { return IFileManager::Get().FileExists(*Path); }));
	UE_LOG(LogIntroFootage, Log, TEXT("Intro footage: %d of %d clips present"), Playlist.Num(), Listed);
	if (Playlist.IsFinished())
	{
		return false;
	}

	Player = NewObject<UMediaPlayer>(this);
	Player->PlayOnOpen = true;
	Player->OnEndReached.AddDynamic(this, &UIntroFootageComponent::HandleEndReached);
	Player->OnMediaOpenFailed.AddDynamic(this, &UIntroFootageComponent::HandleOpenFailed);

	Texture = NewObject<UMediaTexture>(this);
	Texture->AutoClear = true;
	Texture->SetMediaPlayer(Player);
	Texture->UpdateResource();

	Sound = NewObject<UMediaSoundComponent>(GetOwner());
	Sound->bIsUISound = true;
	Sound->SetMediaPlayer(Player);
	Sound->RegisterComponent();

	Brush = FSlateBrush();
	Brush.SetResourceObject(Texture);
	Brush.ImageSize = FVector2D(1280.0, 720.0);

	Overlay = SNew(SBorder)
				  .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				  .BorderBackgroundColor(FLinearColor::Black)
				  .Padding(0.0f)
				  .HAlign(HAlign_Fill)
				  .VAlign(VAlign_Fill)
					  [SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[SNew(SImage).Image(&Brush)]];
	Viewport->AddViewportWidgetContent(Overlay.ToSharedRef(), 1000);

	bPlaying = true;
	SetPlayerControl(false);
	SetComponentTickEnabled(true);
	OpenCurrent();
	return true;
}

void UIntroFootageComponent::OpenCurrent()
{
	const FFootageClip* Clip = Playlist.Current();
	if (!Clip)
	{
		Finish();
		return;
	}
	UFileMediaSource* Source = NewObject<UFileMediaSource>(this);
	Source->SetFilePath(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / Clip->File));
	UE_LOG(LogIntroFootage, Log, TEXT("Playing %s (%s)"), *Clip->File, *Clip->BeatId.ToString());
	if (!Player->OpenSource(Source))
	{
		UE_LOG(LogIntroFootage, Warning, TEXT("Could not open %s; skipping it"), *Clip->File);
		bAdvancePending = true;
	}
}

void UIntroFootageComponent::HandleEndReached()
{
	// Called from the media player's own event: defer to the next tick.
	bAdvancePending = bPlaying;
}

void UIntroFootageComponent::AdvanceClip()
{
	bAdvancePending = false;
	if (!bPlaying)
	{
		return;
	}
	if (Playlist.Advance())
	{
		OpenCurrent();
	}
	else
	{
		Finish();
	}
}

void UIntroFootageComponent::HandleOpenFailed(FString FailedUrl)
{
	UE_LOG(LogIntroFootage, Warning, TEXT("Could not open %s; skipping it"), *FailedUrl);
	bAdvancePending = bPlaying;
}

void UIntroFootageComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bAdvancePending)
	{
		AdvanceClip();
	}
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!bPlaying || !PC)
	{
		return;
	}
	if (PC->WasInputKeyJustPressed(EKeys::Escape) || PC->WasInputKeyJustPressed(EKeys::Gamepad_Special_Right))
	{
		SkipAll();
	}
	else if (PC->WasInputKeyJustPressed(EKeys::SpaceBar) || PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom))
	{
		if (Player)
		{
			Player->Close();
		}
		AdvanceClip();
	}
}

void UIntroFootageComponent::SkipAll()
{
	if (!bPlaying)
	{
		return;
	}
	Playlist.SkipAll();
	Finish();
}

void UIntroFootageComponent::Finish()
{
	if (!bPlaying)
	{
		return;
	}
	bPlaying = false;
	bAdvancePending = false;
	UE_LOG(LogIntroFootage, Log, TEXT("Intro finished; returning control"));
	SetComponentTickEnabled(false);
	if (Player)
	{
		Player->OnEndReached.RemoveAll(this);
		Player->OnMediaOpenFailed.RemoveAll(this);
		Player->Close();
	}
	if (Sound)
	{
		Sound->DestroyComponent();
		Sound = nullptr;
	}
	UWorld* World = GetWorld();
	if (UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr; Viewport && Overlay.IsValid())
	{
		Viewport->RemoveViewportWidgetContent(Overlay.ToSharedRef());
	}
	Overlay.Reset();
	SetPlayerControl(true);
	OnFinished.Broadcast();
}

void UIntroFootageComponent::SetPlayerControl(bool bEnable)
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}
	if (bEnable)
	{
		PC->ResetIgnoreMoveInput();
		PC->ResetIgnoreLookInput();
	}
	else
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}
}

void UIntroFootageComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (bPlaying)
	{
		// Tear down quietly: nothing should start the slice while the world is going away.
		OnFinished.Clear();
		Finish();
	}
	Super::EndPlay(Reason);
}
