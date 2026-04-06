using UnrealBuildTool;

public class ASApiQueryEditor : ModuleRules
{
	public ASApiQueryEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AngelscriptCode",
			"UnrealEd",
			"EditorSubsystem",
			"HTTP",
			"HTTPServer",
			"Json",
			"JsonUtilities",
			"Projects",
		});
	}
}
