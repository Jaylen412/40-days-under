#include "Interaction/SliceInteractorComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Interaction/SliceInteractable.h"

USliceInteractorComponent::USliceInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USliceInteractorComponent::SetInputHeld(ESliceInput Input, bool bHeld)
{
	const int32 Index = static_cast<int32>(Input);
	if (bHeld && !Held[Index])
	{
		Pressed[Index] = true;
	}
	Held[Index] = bHeld;
}

ASliceInteractable* USliceInteractorComponent::FindFocused() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const UWorld* World = GetWorld();
	if (!Pawn || !World)
	{
		return nullptr;
	}
	FVector Origin;
	FRotator Rotation;
	if (const AController* Controller = Pawn->GetController())
	{
		Controller->GetPlayerViewPoint(Origin, Rotation);
	}
	else
	{
		Pawn->GetActorEyesViewPoint(Origin, Rotation);
	}
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SliceInteract), false, Pawn);
	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(Hit, Origin, Origin + Rotation.Vector() * TraceDistanceCm, ECC_Visibility, Params))
	{
		return nullptr;
	}
	ASliceInteractable* Target = Cast<ASliceInteractable>(Hit.GetActor());
	return Target && Hit.Distance <= Target->MaxDistanceCm ? Target : nullptr;
}

void USliceInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
											  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ASliceInteractable* Hit = FindFocused();
	Focused = Hit;
	for (TActorIterator<ASliceInteractable> It(GetWorld()); It; ++It)
	{
		if (It->IsLive())
		{
			const int32 Index = static_cast<int32>(It->Input);
			It->Advance(DeltaTime, !It->bRequiresFocus || *It == Hit, Held[Index], Pressed[Index]);
		}
	}
	Pressed[0] = Pressed[1] = false;
}
