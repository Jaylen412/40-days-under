#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Act2StateMachine.generated.h"

UENUM(BlueprintType)
enum class EAct2State : uint8
{
	Act1,
	SupplyBoatOverdue,
	DistressIntercepted,
	A_Broadcasting,
	A_FamilyDocked,
	A_Siege48h,
	A_EndingRescued,
	A_EndingScuttled,
	B_Preparing,
	B_Swim,
	B_Siphon,
	B_Return,
	B_EndingAlone,
	Dead
};

UENUM(BlueprintType)
enum class EPrepTask : uint8
{
	EmergencyDeco,
	TetherHose600m,
	DecoyBeacon
};

/** Narrative-only state: callers provide day changes, events and elapsed game seconds. */
UCLASS(BlueprintType)
class FORTYDAYSUNDER_API UAct2StateMachine : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Narrative")
	EAct2State GetState() const
	{
		return State;
	}
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void OnDayChanged(int32 Day);
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	bool ChooseBroadcast();
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	bool ChooseGoAlone();
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	bool CompletePrepTask(EPrepTask Task);
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	bool Advance(FName Event);
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void NotifyDeath();
	UFUNCTION(BlueprintPure, Category = "Narrative")
	double SiegeHoursRemaining() const
	{
		return SiegeSeconds / 3600.0;
	}
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void TickSiege(double GameSeconds);

private:
	EAct2State State = EAct2State::Act1;
	uint8 PrepMask = 0;
	double SiegeSeconds = 0.0;
};
