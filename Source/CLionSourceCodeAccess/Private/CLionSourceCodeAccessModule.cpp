// Copyright 2017 dotBunny Inc. All Rights Reserved.

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "Runtime/Core/Public/Features/IModularFeatures.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "CLionSourceCodeAccessModule.h"
#include "CLionSettings.h"

#define LOCTEXT_NAMESPACE "CLionSourceCodeAccessor"

IMPLEMENT_MODULE(FCLionSourceCodeAccessModule, CLionSourceCodeAccess);

void FCLionSourceCodeAccessModule::AddMenuOptions(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("CLionMenu", LOCTEXT("CLionMenuLabel", "CLion"));

	MenuBuilder.AddMenuEntry(
			LOCTEXT("CLionMenuGenerateCMakeListLabel", "Generate CMakeList"),
			LOCTEXT("CLionMenuGenerateCMakeListTooltip", "Generates the CMakeList file for the opened project."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FCLionSourceCodeAccessModule::HandleGenerateProjectFiles)));

	MenuBuilder.AddMenuEntry(
			LOCTEXT("CLionMenuOpenCLionLabel", "Open CLion"),
			LOCTEXT("CLionMenuOpenCLionTooltip", "Generates the CMakeList file, and opens CLion."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FCLionSourceCodeAccessModule::HandleOpenCLion)));

	MenuBuilder.EndSection();
}

void FCLionSourceCodeAccessModule::HandleGenerateProjectFiles()
{
	CLionSourceCodeAccessor.GenerateProjectFile();
}

void FCLionSourceCodeAccessModule::HandleOpenCLion()
{
	CLionSourceCodeAccessor.OpenSolution();
}

void FCLionSourceCodeAccessModule::RegisterMenu()
{
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(
				TEXT("LevelEditor"));

		MainMenuExtender = MakeShareable(new FExtender);
		MainMenuExtender->AddMenuExtension("FileProject", EExtensionHook::After, NULL,
		                                   FMenuExtensionDelegate::CreateRaw(this,
		                                                                     &FCLionSourceCodeAccessModule::AddMenuOptions));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MainMenuExtender);
	}

}

void FCLionSourceCodeAccessModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "CLion", LOCTEXT("RuntimeSettingsName", "CLion"),
		                                 LOCTEXT("RuntimeSettingsDescription", "Configure the CLion Integration"),
		                                 GetMutableDefault<UCLionSettings>());
	}
}

void FCLionSourceCodeAccessModule::ShutdownModule()
{
	CLionSourceCodeAccessor.Shutdown();

	if (UObjectInitialized())
	{
		this->UnregisterSettings();
	}


	// unbind provider from editor
	IModularFeatures::Get().UnregisterModularFeature(TEXT("SourceCodeAccessor"), &CLionSourceCodeAccessor);
}

void FCLionSourceCodeAccessModule::StartupModule()
{

	// Register our custom settings
	this->RegisterSettings();

	// Register our custom menu additions
	this->RegisterMenu();

	// Start her up
	CLionSourceCodeAccessor.Startup();

	// Bind our source control provider to the editor
	IModularFeatures::Get().RegisterModularFeature(TEXT("SourceCodeAccessor"), &CLionSourceCodeAccessor);
}

bool FCLionSourceCodeAccessModule::SupportsDynamicReloading()
{
	// TODO: Until we have this all fixed up
	return false;
}

void FCLionSourceCodeAccessModule::UnregisterSettings()
{
	// Ensure to unregister all of your registered settings here, hot-reload would
	// otherwise yield unexpected results.
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "CLionSettings", "General");
	}
}

#undef LOCTEXT_NAMESPACE