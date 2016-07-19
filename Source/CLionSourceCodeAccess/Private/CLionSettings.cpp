// Copyright 2016 dotBunny, Inc. All Rights Reserved.

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSettings.h"

#define LOCTEXT_NAMESPACE "CLionSourceCodeAccessor"

// We cant have # comments in the template as it gets saved in an ini and it screws that up royally
static const char* CMakeListDefault =
        // Establish the minimum version of CMake
        "cmake_minimum_required (VERSION 3.3)\n"

        // Name the project the normal generated project name
        "project (UE4)\n"

        // Define the path to the build tool
        "set(BUILD mono \"<<UE_BUILDTOOL>>\")\n"

        // Set Platform Flag (for path mapping)
        "set(platform \"<<PLATFORM>>\")\n

        // Define Header Search Macro
        "macro(HEADER_DIRECTORIES base_path return_list)\n"
        "   file(GLOB_RECURSE new_list ${base_path}/*.h)\n"
        "   string(LENGTH ${base_path} base_length)\n"
        "   set(dir_list \"\")\n"
        "   foreach(file_path ${new_list})\n"
        "       get_filename_component(dir_path ${file_path} DIRECTORY)\n"
        "       if (NOT ${dir_path} IN_LIST dir_list)\n"
        "           SET(dir_list ${dir_list} ${dir_path})\n"
        "       endif()\n"
        "       set(loop 1)\n"
        "       set(parent_dir ${dir_path})\n"
        "       while(loop GREATER 0)\n"
        "           get_filename_component(parent_dir ${parent_dir} DIRECTORY)\n"
        "           string(LENGTH ${parent_dir} parent_length)\n"
        "           if (NOT parent_length LESS base_length)\n"
        "               if (NOT ${parent_dir} IN_LIST dir_list)\n"
        "                   set(dir_list ${dir_list} ${parent_dir})\n"
        "               endif()\n"
        "           else()\n"
        "               set(loop 0)\n"
        "           endif()\n"
        "       endwhile(loop GREATER 0)\n"
        "   endforeach()\n"
        "   set(${return_list} ${dir_list})\n"
        "endmacro()\n"

        // Determine directories we're going to include for reference\n"
        "HEADER_DIRECTORIES(\"<<PROJECT_ROOT>>\" ProjectHeaders)\n"

        "HEADER_DIRECTORIES(\"<<UE_SOURCE>>\" UnrealSource)\n"
        "HEADER_DIRECTORIES(\"<<UE_PLUGINS>>\" UnrealPlugins)\n"
        "HEADER_DIRECTORIES(\"<<UE_INTERMEDIATE>>Build<<DIRECTORY_SEPARATOR>><<PLATFORM_CODE>>\" UnrealIntermediate)\n"
        "include_directories(${UnrealSource})\n"
        "include_directories(${UnrealPlugins})\n"
        "include_directories(${UnrealIntermediate})\n"

        "include_directories(${ProjectHeaders})\n"

//        "MACRO(CONFIG_FILES base_path return_list)\n"
//        "ENDMACRO()\n"


        // Handle Our Project Files Specifically
        "file(GLOB_RECURSE Source Source<<DIRECTORY_SEPARATOR>>*.cpp)\n"
        "file(GLOB_RECURSE SourceHeaders Source<<DIRECTORY_SEPARATOR>>*.h)\n"
        "file(GLOB_RECURSE Plugins Plugins<<DIRECTORY_SEPARATOR>>*.cpp)\n"
        "file(GLOB_RECURSE PluginsHeaders Plugins<<DIRECTORY_SEPARATOR>>*.h)\n"
        "file(GLOB_RECURSE Intermediate Intermediate<<DIRECTORY_SEPARATOR>>Build<<DIRECTORY_SEPARATOR>><<PLATFORM_CODE>><<DIRECTORY_SEPARATOR>>*.cpp)\n"
        "file(GLOB_RECURSE IntermediateHeaders Intermediate<<DIRECTORY_SEPARATOR>>Build<<DIRECTORY_SEPARATOR>><<PLATFORM_CODE>><<DIRECTORY_SEPARATOR>>*.h)\n"

//        "add_custom_target(UE4Editor-<<PLATFORM>>-DebugGame ${BUILD}  UE4Editor <<PLATFORM>> DebugGame $(ARGS))\n"
//        "add_custom_target(UE4Editor-<<PLATFORM>>-Shipping ${BUILD}  UE4Editor <<PLATFORM>> Shipping $(ARGS))\n"
//        "add_custom_target(UE4Editor ${BUILD}  UE4Editor <<PLATFORM>> Development $(ARGS) SOURCES ${SOURCE_FILES} ${HEADER_FILES} ${CONFIG_FILES})\n"
//        "add_custom_target(UE4Game-<<PLATFORM>>-DebugGame ${BUILD}  UE4Game <<PLATFORM>> DebugGame $(ARGS))\n"
//        "add_custom_target(UE4Game-<<PLATFORM>>-Shipping ${BUILD}  UE4Game <<PLATFORM>> Shipping $(ARGS))\n"
//        "add_custom_target(UE4Game ${BUILD}  UE4Game <<PLATFORM>> Development $(ARGS) SOURCES ${SOURCE_FILES} ${HEADER_FILES} ${CONFIG_FILES})\n"
//        "add_custom_target(<<PROJECT_NAME>>-<<PLATFORM>>-DebugGame ${BUILD}  -project=\"<<PROJECT_FILE>>\" <<PROJECT_NAME>> <<PLATFORM>> DebugGame $(ARGS))\n"
//        "add_custom_target(<<PROJECT_NAME>>-<<PLATFORM>>-Shipping ${BUILD}  -project=\"<<PROJECT_FILE>>\" <<PROJECT_NAME>> <<PLATFORM>> Shipping $(ARGS))\n"
//        "add_custom_target(<<PROJECT_NAME>> ${BUILD}  -project=\"<<PROJECT_FILE>>\" <<PROJECT_NAME>> <<PLATFORM>> Development $(ARGS) SOURCES ${SOURCE_FILES} ${HEADER_FILES} ${CONFIG_FILES})\n"
//        "add_custom_target(<<PROJECT_NAME>>Editor-<<PLATFORM>>-DebugGame ${BUILD}  -project=\"<<PROJECT_FILE>>\" <<PROJECT_NAME>>Editor <<PLATFORM>> DebugGame $(ARGS))\n"
//        "add_custom_target(<<PROJECT_NAME>>Editor-<<PLATFORM>>-Shipping ${BUILD}  -project=\"<<PROJECT_FILE>>\" <<PROJECT_NAME>>Editor <<PLATFORM>> Shipping $(ARGS))\n"
//        "add_custom_target(<<PROJECT_NAME>>Editor ${BUILD}  -project=\"<<PROJECT_FILE>>\" <<PROJECT_NAME>>Editor <<PLATFORM>> Development $(ARGS) SOURCES ${SOURCE_FILES} ${HEADER_FILES} ${CONFIG_FILES})\n"

        "add_executable(UnrealEngine ${Source} ${SourceHeaders} ${Intermediate} ${IntermediateHeaders} ${Plugins} ${PluginsHeaders})\n";


UCLionSettings::UCLionSettings(const FObjectInitializer& ObjectInitializer)
        : Super(ObjectInitializer)
{
    // TODO: Default Values

    // Project Folder
    TCHAR ConfigProjectPath[MAX_PATH];
    FPlatformMisc::GetEnvironmentVariable(TEXT("CLIONPROJECTPATH"), ConfigProjectPath, MAX_PATH);
    this->ProjectPath.Path = FString(ConfigProjectPath);

    // Unreal Source Folder
    TCHAR ConfigEnginePath[MAX_PATH];
    FPlatformMisc::GetEnvironmentVariable(TEXT("CLIONENGINEPATH"), ConfigEnginePath, MAX_PATH);
    this->EnginePath.Path = FString(ConfigEnginePath);

    // CLang++ Path
    TCHAR ConfigCLangXXPath[MAX_PATH];
    FPlatformMisc::GetEnvironmentVariable(TEXT("CLANGXXPATH"), ConfigCLangXXPath, MAX_PATH);
    this->CLangXXPath.FilePath = FString(ConfigCLangXXPath);

    USceneComponent->dfd
    // CLang Path
    TCHAR ConfigCLangPath[MAX_PATH];
    FPlatformMisc::GetEnvironmentVariable(TEXT("CLANGPATH"), ConfigCLangPath, MAX_PATH);
    this->CLangPath.FilePath = FString(ConfigCLangPath);

    // CLion App Path
    TCHAR ConfigPath[MAX_PATH];
    FPlatformMisc::GetEnvironmentVariable(TEXT("CLIONPATH"), ConfigPath, MAX_PATH);
    this->CLionPath.FilePath = FString(ConfigPath);
    if ( this->CLionPath.FilePath.IsEmpty())
    {
#if PLATFORM_MAC
        this->CLionPath.FilePath = TEXT("/Applications/CLion.app/Contents/MacOS/clion");
#elif PLATFORM_WINDOWS
        this->CLionPath.FilePath = TEXT("C:\\Program Files (x86)\\JetBrains\\CLion 2016.1\\bin\\clion64.exe");
#endif
    }


    // Handle Template
    TCHAR MakeListTemplate[2000];
    FPlatformMisc::GetEnvironmentVariable(TEXT("CLIONMAKELIST"), MakeListTemplate, 2000);
    FString MakeListString = FString(MakeListTemplate);
    if ( MakeListString.IsEmpty() )
    {
        MakeListString = FString(CMakeListDefault);
    }
    this->CMakeTemplate = FText::FromString(MakeListString);



    this->bRequestRefresh = false;

    // Check Setup
    this->CheckSetup();
}

bool UCLionSettings::CheckSetup()
{
    if ( this->ProjectPath.Path.IsEmpty() ||
         this->EnginePath.Path.IsEmpty() ||
         this->CLionPath.FilePath.IsEmpty())
    {
        this->bSetupComplete = false;
    }
    this->bSetupComplete = true;

    return this->bSetupComplete;
}

#if WITH_EDITOR
void UCLionSettings::PreEditChange(UProperty* PropertyAboutToChange)
{
    PreviousEnginePath = this->EnginePath.Path;
    PreviousProjectPath = this->ProjectPath.Path;
    PreviousCLionPath = this->CLionPath.FilePath;
    PreviousCLangPath = this->CLangPath.FilePath;
    PreviousCLangXXPath = this->CLangXXPath.FilePath;
}

void UCLionSettings::PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent )
{
    const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    const FName MemberPropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

    if ( MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, ProjectPath) )
    {
        ProjectPath.Path = FPaths::ConvertRelativePathToFull(ProjectPath.Path);
        ProjectPath.Path = ProjectPath.Path.Trim();
        ProjectPath.Path = ProjectPath.Path.TrimTrailing();

        if ( ProjectPath.Path == PreviousProjectPath ) return;

        FText FailReason;
        if (!FPaths::ValidatePath(ProjectPath.Path, &FailReason))
        {
            FMessageDialog::Open(EAppMsgType::Ok, FailReason);
            ProjectPath.Path = PreviousProjectPath;
            return;
        }

        if (!FPaths::DirectoryExists(ProjectPath.Path))
        {
            FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Please enter a valid project path"));
            ProjectPath.Path = PreviousProjectPath;
            return;
        }

        bRequestRefresh = true;
    }

    if ( MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, EnginePath) )
    {
        EnginePath.Path = FPaths::ConvertRelativePathToFull(EnginePath.Path);
        EnginePath.Path = EnginePath.Path.Trim();
        EnginePath.Path = EnginePath.Path.TrimTrailing();

        if ( EnginePath.Path == PreviousEnginePath ) return;

        FText FailReason;
        if (!FPaths::ValidatePath(EnginePath.Path, &FailReason))
        {
            FMessageDialog::Open(EAppMsgType::Ok, FailReason);
            EnginePath.Path = PreviousEnginePath;
            return;
        }

        if (!FPaths::DirectoryExists(EnginePath.Path))
        {
            FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Please enter a valid engine path"));
            EnginePath.Path = PreviousEnginePath;
            return;
        }

        bRequestRefresh = true;
    }

    if ( MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, CLangPath) )
    {
        CLangPath.FilePath = FPaths::ConvertRelativePathToFull(CLangPath.FilePath);
        CLangPath.FilePath = CLangPath.FilePath.Trim();
        CLangPath.FilePath = CLangPath.FilePath.TrimTrailing();

        if ( CLangPath.FilePath == PreviousCLangPath ) return;

        FText FailReason;
        if (!FPaths::ValidatePath(CLangPath.FilePath, &FailReason))
        {
            FMessageDialog::Open(EAppMsgType::Ok, FailReason);
            CLangPath.FilePath = PreviousCLangPath;
            return;
        }

        bRequestRefresh = true;
    }

    if ( MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, CLangXXPath) )
    {
        CLangXXPath.FilePath = FPaths::ConvertRelativePathToFull(CLangXXPath.FilePath);
        CLangXXPath.FilePath = CLangXXPath.FilePath.Trim();
        CLangXXPath.FilePath = CLangXXPath.FilePath.TrimTrailing();

        if ( CLangXXPath.FilePath == PreviousCLangXXPath ) return;

        FText FailReason;
        if (!FPaths::ValidatePath(CLangXXPath.FilePath, &FailReason))
        {
            FMessageDialog::Open(EAppMsgType::Ok, FailReason);
            CLangXXPath.FilePath = PreviousCLangXXPath;
            return;
        }

        bRequestRefresh = true;
    }

    if ( MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings,CLionPath) )
    {
        CLionPath.FilePath = FPaths::ConvertRelativePathToFull(CLionPath.FilePath);
        CLionPath.FilePath = CLionPath.FilePath.Trim();
        CLionPath.FilePath = CLionPath.FilePath.TrimTrailing();

        FText FailReason;

#if PLATFORM_MAC
        if ( CLionPath.FilePath.EndsWith(TEXT("clion.app"))) {
            CLionPath.FilePath = CLionPath.FilePath.Append(TEXT("/Contents/MacOS/clion"));
        }

        if ( !CLionPath.FilePath.Contains(TEXT("clion.app")))
        {
            FailReason = LOCTEXT( "CLionSelectMacApp", "Please select the CLion app" );
            FMessageDialog::Open(EAppMsgType::Ok, FailReason);
            CLionPath.FilePath = PreviousCLionPath;
            return;
        }
#endif

        if ( CLionPath.FilePath == PreviousCLionPath ) return;


        if (!FPaths::ValidatePath(CLionPath.FilePath, &FailReason))
        {
            FMessageDialog::Open(EAppMsgType::Ok, FailReason);
            CLionPath.FilePath = PreviousCLionPath;
            return;
        }

        bRequestRefresh = true;
    }

    this->CheckSetup();
}
#endif


bool UCLionSettings::IsSetup()
{
    return this->bSetupComplete;
}

bool UCLionSettings::OutputCMakeList()
{
    // mono /Users/Shared/UnrealEngine/4.12/Engine/Binaries/DotNET/UnrealBuildTool.exe "/Users/reapazor/Workspaces/dotBunny/Dethol/Game/Project/Dethol".uproject -Game Dethol -OnlyPublic -CMakeFile -CurrentPlatform -NoShippingConfigs

    // Create our directory separator based on our platform
    // Untest on non mac platforms
#if PLATFORM_WINDOWS
    FString DirectorySeparator = TEXT("\");
    FString PlatformName = TEXT("Windows");
    FString PlatformCode = TEXT("Win64");
#elif PLATFORM_LINUX
    FString DirectorySeparator = TEXT("/");
    FString PlatformName = TEXT("Linux");
    FString PlatformCode = TEXT("Linux");
#else
    FString DirectorySeparator = TEXT("/");
    FString PlatformName = TEXT("Mac");
    FString PlatformCode = TEXT("Mac");
#endif


    // Make a local copy of our template file
    FString OutputTemplate = this->CMakeTemplate.ToString();
    FString OutputPath = this->ProjectPath.Path + DirectorySeparator + "CMakeLists.txt";

    // Handle Project Folder
    OutputTemplate = OutputTemplate.Replace(TEXT("<<PROJECT_ROOT>>"), *this->ProjectPath.Path);

    // Handle Engine Folders
    OutputTemplate = OutputTemplate.Replace(TEXT("<<UE_ROOT>>"), *this->EnginePath.Path);

    FString UnrealSourcePath = this->EnginePath.Path + DirectorySeparator + TEXT("Source");
    OutputTemplate = OutputTemplate.Replace(TEXT("<<UE_SOURCE>>"), *UnrealSourcePath);

    FString UnrealPluginsPath = this->EnginePath.Path + DirectorySeparator + TEXT("Plugins");
    OutputTemplate = OutputTemplate.Replace(TEXT("<<UE_PLUGINS>>"), *UnrealPluginsPath);

    FString UnrealIntermediatePath = this->EnginePath.Path + DirectorySeparator + TEXT("Intermediate");
    OutputTemplate = OutputTemplate.Replace(TEXT("<<UE_INTERMEDIATE>>"), *UnrealIntermediatePath);

    // Unreal Build Tool Path
    FString UnrealBuildToolPath = this->EnginePath.Path + DirectorySeparator + TEXT("Binaries") + DirectorySeparator + TEXT("DotNET") + DirectorySeparator + TEXT("UnrealBuildTool.exe");
    OutputTemplate = OutputTemplate.Replace(TEXT("<<UE_BUILDTOOL>>"), *UnrealBuildToolPath);

    // Handle CLang++ / CLang (If Defined)
    if ( !this->CLangXXPath.FilePath.IsEmpty() )
    {
        FString CLangXXSetting = TEXT("set(CMAKE_CXX_COMPILER \"");
        CLangXXSetting += this->CLangXXPath.FilePath;
        CLangXXSetting += "\")\n";
        OutputTemplate = OutputTemplate.Append(CLangXXSetting);
    }
    if ( !this->CLangPath.FilePath.IsEmpty() )
    {
        FString CLangSetting = TEXT("set(CMAKE_C_COMPILER \"");
        CLangSetting += this->CLangPath.FilePath;
        CLangSetting += "\")\n";
        OutputTemplate = OutputTemplate.Append(CLangSetting);
    }

    // Platform Specific
    OutputTemplate = OutputTemplate.Replace(TEXT("<<DIRECTORY_SEPARATOR>>")*DirectorySeparator);
    OutputTemplate = OutputTemplate.Replace(TEXT("<<PLATFORM>>"),*PlatformName);
    OutputTemplate = OutputTemplate.Replace(TEXT("<<PLATFORM_CODE>>"),*PlatformCode);


    // Write out the file
    if (FFileHelper::SaveStringToFile(OutputTemplate, *OutputPath,  FFileHelper::EEncodingOptions::Type::ForceAnsi)) {
        return true;
    }

    return false;
}