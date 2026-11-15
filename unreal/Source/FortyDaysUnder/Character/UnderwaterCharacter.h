#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/ViewmodelArm.h"
#include "Simulation/VitalsSnapshot.h"
#include "UnderwaterCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USurvivalVitalsComponent;
class UInputAction;
class UInputMappingContext;
class USliceInteractorComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
struct FInputActionValue;

UCLASS()
class FORTYDAYSUNDER_API AUnderwaterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AUnderwaterCharacter(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere) TObjectPtr<USurvivalVitalsComponent> Vitals;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> FirstPersonCamera;
	UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> ThirdPersonBoom;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> ThirdPersonCamera;

	UPROPERTY(EditAnywhere, Category="Input") TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, Category="Input") TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditAnywhere, Category="Input") TObjectPtr<UInputAction> AscendAction;
	UPROPERTY(EditAnywhere, Category="Input") TObjectPtr<UInputAction> ToggleCameraAction;
	UPROPERTY(EditAnywhere, Category="Input") TObjectPtr<UInputAction> InteractAction;
	UPROPERTY(EditAnywhere, Category="Input") TObjectPtr<UInputAction> WatchAction;

	/** Opening-slice interactions: view trace plus Interact / Watch input. */
	UPROPERTY(VisibleAnywhere) TObjectPtr<USliceInteractorComponent> Interactor;

	/**
	 * First-person forearms (owner-only, no shadows), parented to the first-person camera. They rest below the
	 * view; holding Watch raises the left wrist to the face, holding Interact reaches the right hand forward.
	 */
	UPROPERTY(VisibleAnywhere, Category="FirstPerson") TObjectPtr<UStaticMeshComponent> ArmLeft;
	UPROPERTY(VisibleAnywhere, Category="FirstPerson") TObjectPtr<UStaticMeshComponent> ArmRight;
	/** Dive watch on the left forearm; its screen shows FWatchDisplay lines as in-world text (no HUD). */
	UPROPERTY(VisibleAnywhere, Category="FirstPerson") TObjectPtr<UStaticMeshComponent> Watch;
	UPROPERTY(VisibleAnywhere, Category="FirstPerson") TArray<TObjectPtr<UTextRenderComponent>> WatchLines;

	/** Arm poses relative to the first-person camera (cm). Rest poses sit below the view. */
	UPROPERTY(EditAnywhere, Category="FirstPerson") FTransform ArmLeftRest;
	UPROPERTY(EditAnywhere, Category="FirstPerson") FTransform ArmLeftRaised;
	UPROPERTY(EditAnywhere, Category="FirstPerson") FTransform ArmRightRest;
	UPROPERTY(EditAnywhere, Category="FirstPerson") FTransform ArmRightRaised;

	/** Current raise progress of each arm, 0..1 (for tests and tuning). */
	double GetArmLeftAlpha() const { return LeftArm.Alpha; }
	double GetArmRightAlpha() const { return RightArm.Alpha; }
	/** Advance the arms; called from Tick, public so tests and editor scripts can drive it without play. */
	UFUNCTION(BlueprintCallable, Category="FirstPerson") void UpdateViewmodel(float DeltaSeconds);
	/** Refresh the watch screen text from the vitals snapshot. */
	UFUNCTION(BlueprintCallable, Category="FirstPerson") void RefreshWatch();

	UFUNCTION(BlueprintCallable) void SetCameraBlend(float Alpha);     // 0 first person, 1 third person
	UFUNCTION(BlueprintCallable) float GetInputLatencySeconds() const; // 0.25 x DcsSeverity
	UFUNCTION(BlueprintCallable) float GetAudioLowPassHz() const;      // lerp(20000, 800, DcsSeverity)
	UFUNCTION(BlueprintCallable) float GetHypoxiaVignette() const;     // 0 Nominal, 0.3 Impaired, 0.6 Hypoxic, 1 Critical/Dead
	/** Inside the lab the character walks; outside it swims (forced submerged, no water volume needed). */
	UFUNCTION(BlueprintCallable) void SetInsidePressurisedLab(bool bInside);
	UFUNCTION(BlueprintCallable) double GetDepthMetres() const;        // (WaterSurfaceZ - ActorZ)/100, min 0

	UPROPERTY(EditAnywhere) double WaterSurfaceZ = 0.0;
	UPROPERTY(EditAnywhere) bool bInsidePressurisedLab = true;         // when true, depth sent to Vitals is 15

	/** Pure functions of a snapshot so they can be tested without a world. */
	static float LatencyFor(const FVitalsSnapshot& Snapshot);
	static float LowPassHzFor(const FVitalsSnapshot& Snapshot);
	static float VignetteFor(const FVitalsSnapshot& Snapshot);
	/** Sign applied to look-up input so that pushing up always looks up, whatever the controller's pitch scale. */
	static float PitchSignFor(bool bLegacyInputScales, float LegacyPitchScale);

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;

protected:
	virtual void BeginPlay() override;

private:
	struct FDelayedInput
	{
		double ReleaseTime = 0.0;
		FVector2D Move = FVector2D::ZeroVector;
		FVector2D Look = FVector2D::ZeroVector;
		float Ascend = 0.0f;
	};

	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnAscend(const FInputActionValue& Value);
	void OnToggleCamera();
	void OnInteractStarted();
	void OnInteractCompleted();
	void OnWatchStarted();
	void OnWatchCompleted();
	/** Any action left unassigned gets a code-built default with keyboard, mouse and gamepad keys. */
	void EnsureDefaultInput();

	UPROPERTY(Transient) TObjectPtr<UInputMappingContext> DefaultInputContext;
	void Enqueue(const FVector2D& Move, const FVector2D& Look, float Ascend);
	void ApplyInput(const FDelayedInput& Input);

	TArray<FDelayedInput> InputQueue;
	float CameraBlend = 0.0f;

	FViewmodelArm LeftArm;
	FViewmodelArm RightArm;
	bool bWatchHeld = false;
	bool bInteractHeld = false;
	double WatchRefreshTimer = 0.0;
};
