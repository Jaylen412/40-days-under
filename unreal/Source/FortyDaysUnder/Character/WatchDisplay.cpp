#include "Character/WatchDisplay.h"

#include "Simulation/BuhlmannModel.h"

double FWatchDisplay::DepthMetres(double AmbientBar)
{
	if (!FMath::IsFinite(AmbientBar))
	{
		return 0.0;
	}
	return FMath::Max(0.0, (AmbientBar - FBuhlmannModel::SurfacePressureBar) / FBuhlmannModel::BarPerMetreFresh);
}

TArray<FString> FWatchDisplay::Lines(const FVitalsSnapshot& Snapshot)
{
	TArray<FString> Out;
	Out.Add(FString::Printf(TEXT("%.1f m"), DepthMetres(Snapshot.AmbientBar)));

	const double Ceiling = DepthMetres(Snapshot.CeilingBar);
	if (Snapshot.bInDecoCycle)
	{
		Out.Add(FString::Printf(TEXT("DECO %.1f h"), FMath::Max(0.0, Snapshot.DecoHoursRemaining)));
	}
	else if (Ceiling >= 0.05)
	{
		Out.Add(FString::Printf(TEXT("CEIL %.1f m"), Ceiling));
	}
	else
	{
		Out.Add(TEXT("NO DECO"));
	}

	const double Seconds = FMath::IsFinite(Snapshot.GameSeconds) ? FMath::Max(0.0, Snapshot.GameSeconds) : 0.0;
	const int64 OfDay = static_cast<int64>(Seconds) % 86400;
	Out.Add(FString::Printf(TEXT("D%d %02d:%02d"), FMath::Max(1, Snapshot.Day), static_cast<int32>(OfDay / 3600),
		static_cast<int32>((OfDay / 60) % 60)));
	return Out;
}
