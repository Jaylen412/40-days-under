#include "Character/UnderwaterCharacter.h"

#include "Camera/CameraComponent.h"
#include "Character/UnderwaterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Interaction/SliceInteractorComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Survival/SurvivalVitalsComponent.h"

namespace
{
	constexpr double LabDepthMetres = 15.0;
	constexpr float MaxLatencySeconds = 0.25f;
}

AUnderwaterCharacter::AUnderwaterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UUnderwaterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// Christian Thomas, 175.26 cm.
	GetCapsuleComponent()->InitCapsuleSize(34.0f, 87.63f);

	Vitals = CreateDefaultSubobject<USurvivalVitalsComponent>(TEXT("Vitals"));

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 75.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	ThirdPersonBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonBoom"));
	ThirdPersonBoom->SetupAttachment(GetCapsuleComponent());
	ThirdPersonBoom->TargetArmLength = 250.0f;
	ThirdPersonBoom->bUsePawnControlRotation = true;

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(ThirdPersonBoom, USpringArmComponent::SocketName);
	ThirdPersonCamera->SetAutoActivate(false);

	Interactor = CreateDefaultSubobject<USliceInteractorComponent>(TEXT("Interactor"));
}

void AUnderwaterCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetInsidePressurisedLab(bInsidePressurisedLab);
	SetCameraBlend(CameraBlend);
}

void AUnderwaterCharacter::SetInsidePressurisedLab(bool bInside)
{
	bInsidePressurisedLab = bInside;
	if (UUnderwaterMovementComponent* Move = Cast<UUnderwaterMovementComponent>(GetCharacterMovement()))
	{
		Move->bForceSubmerged = !bInside;
		if (GetWorld())
		{
			Move->SetMovementMode(bInside ? MOVE_Walking : MOVE_Swimming);
		}
	}
	if (bInside)
	{
		Vitals->SetDepthMetres(LabDepthMetres);
	}
}

void AUnderwaterCharacter::SetCameraBlend(float Alpha)
{
	CameraBlend = FMath::Clamp(Alpha, 0.0f, 1.0f);
	const bool bThird = CameraBlend >= 0.5f;
	FirstPersonCamera->SetActive(!bThird);
	ThirdPersonCamera->SetActive(bThird);
}

float AUnderwaterCharacter::LatencyFor(const FVitalsSnapshot& Snapshot)
{
	return MaxLatencySeconds * static_cast<float>(FMath::Clamp(Snapshot.DcsSeverity, 0.0, 1.0));
}

float AUnderwaterCharacter::LowPassHzFor(const FVitalsSnapshot& Snapshot)
{
	return FMath::Lerp(20000.0f, 800.0f, static_cast<float>(FMath::Clamp(Snapshot.DcsSeverity, 0.0, 1.0)));
}

float AUnderwaterCharacter::VignetteFor(const FVitalsSnapshot& Snapshot)
{
	switch (Snapshot.AirStage)
	{
	case EAirStage::Nominal:  return 0.0f;
	case EAirStage::Impaired: return 0.3f;
	case EAirStage::Hypoxic:  return 0.6f;
	default:                  return 1.0f;   // Critical, Dead
	}
}

float AUnderwaterCharacter::GetInputLatencySeconds() const { return LatencyFor(Vitals->GetSnapshot()); }
float AUnderwaterCharacter::GetAudioLowPassHz() const { return LowPassHzFor(Vitals->GetSnapshot()); }
float AUnderwaterCharacter::GetHypoxiaVignette() const { return VignetteFor(Vitals->GetSnapshot()); }

double AUnderwaterCharacter::GetDepthMetres() const
{
	return FMath::Max(0.0, (WaterSurfaceZ - GetActorLocation().Z) / 100.0);
}

void AUnderwaterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bInsidePressurisedLab)
	{
		Vitals->SetDepthMetres(GetDepthMetres());
	}

	if (UUnderwaterMovementComponent* Move = Cast<UUnderwaterMovementComponent>(GetCharacterMovement()))
	{
		const FVitalsSnapshot& S = Vitals->GetSnapshot();
		Move->MaxSwimSpeed = Move->GetEffectiveSwimSpeed(static_cast<float>(S.Stamina), S.AirStage);
	}

	const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	int32 Released = 0;
	while (Released < InputQueue.Num() && InputQueue[Released].ReleaseTime <= Now)
	{
		ApplyInput(InputQueue[Released]);
		++Released;
	}
	InputQueue.RemoveAt(0, Released, EAllowShrinking::No);
}

void AUnderwaterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		return;
	}
	EnsureDefaultInput();
	if (MoveAction)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUnderwaterCharacter::OnMove);
	}
	if (LookAction)
	{
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUnderwaterCharacter::OnLook);
	}
	if (AscendAction)
	{
		EIC->BindAction(AscendAction, ETriggerEvent::Triggered, this, &AUnderwaterCharacter::OnAscend);
	}
	if (ToggleCameraAction)
	{
		EIC->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &AUnderwaterCharacter::OnToggleCamera);
	}
	if (InteractAction)
	{
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AUnderwaterCharacter::OnInteractStarted);
		EIC->BindAction(InteractAction, ETriggerEvent::Completed, this, &AUnderwaterCharacter::OnInteractCompleted);
		EIC->BindAction(InteractAction, ETriggerEvent::Canceled, this, &AUnderwaterCharacter::OnInteractCompleted);
	}
	if (WatchAction)
	{
		EIC->BindAction(WatchAction, ETriggerEvent::Started, this, &AUnderwaterCharacter::OnWatchStarted);
		EIC->BindAction(WatchAction, ETriggerEvent::Completed, this, &AUnderwaterCharacter::OnWatchCompleted);
		EIC->BindAction(WatchAction, ETriggerEvent::Canceled, this, &AUnderwaterCharacter::OnWatchCompleted);
	}
}

void AUnderwaterCharacter::EnsureDefaultInput()
{
	if (DefaultInputContext)
	{
		return;
	}
	DefaultInputContext = NewObject<UInputMappingContext>(this, TEXT("DefaultInputContext"));
	UInputMappingContext* Context = DefaultInputContext;
	// Returns the action to map, creating it only when the designer left the slot empty.
	auto Default = [this](TObjectPtr<UInputAction>& Slot, const TCHAR* Name, EInputActionValueType Type) -> UInputAction*
	{
		if (Slot)
		{
			return nullptr;
		}
		Slot = NewObject<UInputAction>(this, Name);
		Slot->ValueType = Type;
		return Slot;
	};
	auto Map = [Context](UInputAction* Action, FKey Key, bool bSwizzle = false, FVector Negate = FVector::ZeroVector,
						 float Scale = 1.0f)
	{
		if (!Action)
		{
			return;
		}
		FEnhancedActionKeyMapping& Mapping = Context->MapKey(Action, Key);
		if (!Negate.IsZero())
		{
			UInputModifierNegate* Mod = NewObject<UInputModifierNegate>(Context);
			Mod->bX = Negate.X != 0.0;
			Mod->bY = Negate.Y != 0.0;
			Mod->bZ = Negate.Z != 0.0;
			Mapping.Modifiers.Add(Mod);
		}
		if (bSwizzle)
		{
			UInputModifierSwizzleAxis* Mod = NewObject<UInputModifierSwizzleAxis>(Context);
			Mod->Order = EInputAxisSwizzle::YXZ;
			Mapping.Modifiers.Add(Mod);
		}
		if (Scale != 1.0f)
		{
			UInputModifierScalar* Mod = NewObject<UInputModifierScalar>(Context);
			Mod->Scalar = FVector(Scale);
			Mapping.Modifiers.Add(Mod);
		}
	};
	const FVector X(1, 0, 0), Y(0, 1, 0);
	// Move: X is strafe right, Y is forward (see ApplyInput).
	UInputAction* Move = Default(MoveAction, TEXT("IA_Default_Move"), EInputActionValueType::Axis2D);
	Map(Move, EKeys::D);
	Map(Move, EKeys::A, false, X);
	Map(Move, EKeys::W, true);
	Map(Move, EKeys::S, true, X);
	Map(Move, EKeys::Gamepad_Left2D);
	// Look: the legacy pitch scale is negative, so Y is flipped to make up mean up.
	UInputAction* Look = Default(LookAction, TEXT("IA_Default_Look"), EInputActionValueType::Axis2D);
	Map(Look, EKeys::Mouse2D, false, Y, 0.04f);
	Map(Look, EKeys::Gamepad_Right2D, false, Y, 0.6f);
	UInputAction* Ascend = Default(AscendAction, TEXT("IA_Default_Ascend"), EInputActionValueType::Axis1D);
	Map(Ascend, EKeys::SpaceBar);
	Map(Ascend, EKeys::LeftControl, false, X);
	UInputAction* Toggle = Default(ToggleCameraAction, TEXT("IA_Default_ToggleCamera"), EInputActionValueType::Boolean);
	Map(Toggle, EKeys::C);
	Map(Toggle, EKeys::Gamepad_RightThumbstick);
	UInputAction* Interact = Default(InteractAction, TEXT("IA_Default_Interact"), EInputActionValueType::Boolean);
	Map(Interact, EKeys::E);
	Map(Interact, EKeys::LeftMouseButton);
	Map(Interact, EKeys::Gamepad_FaceButton_Bottom);
	UInputAction* Watch = Default(WatchAction, TEXT("IA_Default_Watch"), EInputActionValueType::Boolean);
	Map(Watch, EKeys::Q);
	Map(Watch, EKeys::Gamepad_FaceButton_Top);
}

void AUnderwaterCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	const APlayerController* PC = Cast<APlayerController>(Controller);
	const ULocalPlayer* LocalPlayer = PC ? PC->GetLocalPlayer() : nullptr;
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	if (Subsystem && DefaultInputContext && !Subsystem->HasMappingContext(DefaultInputContext))
	{
		Subsystem->AddMappingContext(DefaultInputContext, 0);
		UE_LOG(LogTemp, Log, TEXT("UnderwaterCharacter: default input context added with %d key mappings"),
			   DefaultInputContext->GetMappings().Num());
	}
}

void AUnderwaterCharacter::OnInteractStarted() { Interactor->SetInputHeld(ESliceInput::Interact, true); }
void AUnderwaterCharacter::OnInteractCompleted() { Interactor->SetInputHeld(ESliceInput::Interact, false); }
void AUnderwaterCharacter::OnWatchStarted() { Interactor->SetInputHeld(ESliceInput::Watch, true); }
void AUnderwaterCharacter::OnWatchCompleted() { Interactor->SetInputHeld(ESliceInput::Watch, false); }

void AUnderwaterCharacter::OnMove(const FInputActionValue& Value) { Enqueue(Value.Get<FVector2D>(), FVector2D::ZeroVector, 0.0f); }
void AUnderwaterCharacter::OnLook(const FInputActionValue& Value) { Enqueue(FVector2D::ZeroVector, Value.Get<FVector2D>(), 0.0f); }
void AUnderwaterCharacter::OnAscend(const FInputActionValue& Value) { Enqueue(FVector2D::ZeroVector, FVector2D::ZeroVector, Value.Get<float>()); }
void AUnderwaterCharacter::OnToggleCamera() { SetCameraBlend(CameraBlend < 0.5f ? 1.0f : 0.0f); }

void AUnderwaterCharacter::Enqueue(const FVector2D& Move, const FVector2D& Look, float Ascend)
{
	FDelayedInput Input;
	Input.ReleaseTime = (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0) + GetInputLatencySeconds();
	Input.Move = Move;
	Input.Look = Look;
	Input.Ascend = Ascend;
	InputQueue.Add(Input);
}

void AUnderwaterCharacter::ApplyInput(const FDelayedInput& Input)
{
	if (!Input.Move.IsNearlyZero())
	{
		const FRotator ControlRot = Controller ? Controller->GetControlRotation() : GetActorRotation();
		AddMovementInput(ControlRot.RotateVector(FVector::ForwardVector), Input.Move.Y);
		AddMovementInput(ControlRot.RotateVector(FVector::RightVector), Input.Move.X);
	}
	if (!Input.Look.IsNearlyZero())
	{
		AddControllerYawInput(Input.Look.X);
		AddControllerPitchInput(Input.Look.Y);
	}
	if (!FMath::IsNearlyZero(Input.Ascend))
	{
		AddMovementInput(FVector::UpVector, Input.Ascend);
	}
}
