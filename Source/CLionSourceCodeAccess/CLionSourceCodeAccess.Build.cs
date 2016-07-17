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
					"DesktopPlatform",

                    "LevelEditor"
				}
			);

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "UnrealEd",
                    "Slate",
                    "SlateCore",
                    "WorkspaceMenuStructure",
                });
		}
	}
}