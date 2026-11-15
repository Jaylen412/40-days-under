#pragma once

#include "CoreMinimal.h"
#include "VitalsSnapshot.generated.h"

/** Breathing-air quality inside Lab 04, driven by CO2 (and O2 for Dead). */
UENUM(BlueprintType)
enum class EAirStage : uint8
{
	Nominal,
	Impaired,  // CO2 >= 3%
	Hypoxic,   // CO2 >= 5%
	Critical,  // CO2 >= 8%
	Dead       // CO2 >= 10% or O2 <= 10%
};

/** The state contract every other system reads. */
USTRUCT(BlueprintType)
struct FVitalsSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) double GameSeconds = 0;
	UPROPERTY(BlueprintReadOnly) int32 Day = 1;                 // 1-based
	UPROPERTY(BlueprintReadOnly) double FuelFraction = 1;
	UPROPERTY(BlueprintReadOnly) bool bPumpRunning = true;
	UPROPERTY(BlueprintReadOnly) bool bPumpSputtering = false;
	UPROPERTY(BlueprintReadOnly) double Co2Pct = 0.04;
	UPROPERTY(BlueprintReadOnly) double O2Pct = 20.9;
	UPROPERTY(BlueprintReadOnly) EAirStage AirStage = EAirStage::Nominal;
	UPROPERTY(BlueprintReadOnly) double AmbientBar = 0;
	UPROPERTY(BlueprintReadOnly) double CeilingBar = 0;
	UPROPERTY(BlueprintReadOnly) double DcsSeverity = 0;         // 0..1
	UPROPERTY(BlueprintReadOnly) bool bInDecoCycle = false;
	UPROPERTY(BlueprintReadOnly) double DecoHoursRemaining = 0;
	UPROPERTY(BlueprintReadOnly) double Health = 100;
	UPROPERTY(BlueprintReadOnly) double Stamina = 100;
	UPROPERTY(BlueprintReadOnly) double HydrationL = 3;
	UPROPERTY(BlueprintReadOnly) double CaloriesKcal = 2400;
	UPROPERTY(BlueprintReadOnly) double Isolation = 0;
	UPROPERTY(BlueprintReadOnly) bool bDead = false;
};
