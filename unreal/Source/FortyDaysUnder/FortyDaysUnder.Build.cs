using UnrealBuildTool;

public class FortyDaysUnder : ModuleRules
{
	public FortyDaysUnder(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Sources include each other by folder, e.g. "Simulation/VitalsSnapshot.h".
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"ApplicationCore",
			"Json",
			"JsonUtilities"
		});
	}
}
