// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class CLionSourceCodeAccess : ModuleRules
	{
        public CLionSourceCodeAccess(TargetInfo Target)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"SourceCodeAccess",
					// "MainFrame",
					"DesktopPlatform",
                    "LevelEditor"
				}
			);

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "Engine",
                    "InputCore",
                    "UnrealEd",
                    "CoreUObject",
                    "Slate",
                    "SlateCore",
                    "WorkspaceMenuStructure",
                    "Projects",
                    "PropertyEditor"
                });
		}
	}
}