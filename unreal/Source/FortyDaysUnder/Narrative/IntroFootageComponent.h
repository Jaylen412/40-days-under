#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Narrative/FootagePlaylist.h"
#include "Styling/SlateBrush.h"
#include "IntroFootageComponent.generated.h"

class SWidget;
class UMediaPlayer;
class UMediaSoundComponent;
class UMediaTexture;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIntroFootageFinished);

/**
 * Plays the Higgsfield realism clips full screen, in manifest order, before the player gets control.
 * A cutscene, not telemetry: nothing is drawn once it ends. Space / gamepad A skips a clip,
 * Escape / gamepad Start skips the rest.
 */
UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class FORTYDAYSUNDER_API UIntroFootageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UIntroFootageComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footage")
	bool bEnabled = true;

	/** Manifest file, relative to the project Content directory. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footage")
	FString ManifestFile = TEXT("Data/Narrative/opening_slice_footage.json");

	/** Starts the intro. False (and no broadcast) when disabled, headless or no clip file exists. */
	UFUNCTION(BlueprintCallable, Category = "Footage")
	bool StartIntro();

	/** Stops playback, gives control back and broadcasts OnFinished. */
	UFUNCTION(BlueprintCallable, Category = "Footage")
	void SkipAll();

	UFUNCTION(BlueprintPure, Category = "Footage")
	bool IsPlaying() const
	{
		return bPlaying;
	}

	UPROPERTY(BlueprintAssignable, Category = "Footage")
	FOnIntroFootageFinished OnFinished;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> Player;
	UPROPERTY(Transient)
	TObjectPtr<UMediaTexture> Texture;
	UPROPERTY(Transient)
	TObjectPtr<UMediaSoundComponent> Sound;

	FFootagePlaylist Playlist;
	FSlateBrush Brush;
	TSharedPtr<SWidget> Overlay;
	bool bPlaying = false;
	/** Set from media-player callbacks; acted on next tick so the player is never closed inside its own event. */
	bool bAdvancePending = false;

	void OpenCurrent();
	void AdvanceClip();
	void Finish();
	void SetPlayerControl(bool bEnable);

	UFUNCTION()
	void HandleEndReached();
	UFUNCTION()
	void HandleOpenFailed(FString FailedUrl);
};
