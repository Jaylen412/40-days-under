#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Simulation/VitalsSnapshot.h"
#include "UnderwaterCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USurvivalVitalsComponent;
class UInputAction;
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

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

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
	void Enqueue(const FVector2D& Move, const FVector2D& Look, float Ascend);
	void ApplyInput(const FDelayedInput& Input);

	TArray<FDelayedInput> InputQueue;
	float CameraBlend = 0.0f;
};
