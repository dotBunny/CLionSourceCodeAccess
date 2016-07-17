// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
// Copyright 2016 dotBunny, Inc. All Rights Reserved.


#include "CLionSourceCodeAccessPrivatePCH.h"
#include "Runtime/Core/Public/Features/IModularFeatures.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "ISettingsModule.h"
#include "CLionSourceCodeAccessModule.h"

#define LOCTEXT_NAMESPACE "CLionSourceCodeAccessor"

IMPLEMENT_MODULE( FCLionSourceCodeAccessModule, CLionSourceCodeAccess );

void FCLionSourceCodeAccessModule::StartupModule()
{
    CLionSourceCodeAccessor.Startup();

	// Bind our source control provider to the editor
	IModularFeatures::Get().RegisterModularFeature(TEXT("SourceCodeAccessor"), &CLionSourceCodeAccessor );

    if ( FModuleManager::Get().IsModuleLoaded( "LevelEditor" ) )
    {
        FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>( TEXT("LevelEditor") );

        MainMenuExtender = MakeShareable(new FExtender);
        MainMenuExtender->AddMenuExtension("FileProject", EExtensionHook::After, NULL, FMenuExtensionDelegate::CreateRaw(this, &FCLionSourceCodeAccessModule::AddMenuOptions));

        LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MainMenuExtender);
    }
}

void FCLionSourceCodeAccessModule::AddMenuOptions(FMenuBuilder& MenuBuilder)
{

    MenuBuilder.BeginSection("CLionMenu", LOCTEXT("CLionMenuLabel", "CLion"));

    MenuBuilder.AddMenuEntry(
            LOCTEXT("CLionMenuGenerateCMakeListLabel", "Generate CMakeList"),
            LOCTEXT("CLionMenuGenerateCMakeListTooltip", "Generates the CMakeList file for the opened project."),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateRaw(this, &FCLionSourceCodeAccessModule::ClickGenerateProjectFiles)));

    MenuBuilder.EndSection();
}


void FCLionSourceCodeAccessModule::ShutdownModule()
{
    CLionSourceCodeAccessor.Shutdown();
	// unbind provider from editor
	IModularFeatures::Get().UnregisterModularFeature(TEXT("SourceCodeAccessor"), &CLionSourceCodeAccessor);
}

void FCLionSourceCodeAccessModule::ClickGenerateProjectFiles()
{
    CLionSourceCodeAccessor.GenerateProjectFile();
}

FCLionSourceCodeAccessor& FCLionSourceCodeAccessModule::GetAccessor()
{
    return CLionSourceCodeAccessor;
}
