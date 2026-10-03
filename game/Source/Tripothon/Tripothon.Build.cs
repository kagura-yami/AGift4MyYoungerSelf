using UnrealBuildTool;
public class Tripothon : ModuleRules
{
    public Tripothon(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicIncludePaths.Add(ModuleDirectory);
        PublicDependencyModuleNames.AddRange(new[] {"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GameplayTags"});
        if (Target.bBuildEditor) PrivateDependencyModuleNames.AddRange(new[] {"MeshDescription", "AssetRegistry"});
        PrivateDependencyModuleNames.AddRange(new[] {"Slate", "SlateCore", "AIModule", "NavigationSystem"});
    }
}
