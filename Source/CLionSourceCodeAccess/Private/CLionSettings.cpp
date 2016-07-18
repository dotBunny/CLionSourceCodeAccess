// Copyright 2016 dotBunny, Inc. All Rights Reserved.

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSettings.h"

#define LOCTEXT_NAMESPACE "CLionSourceCodeAccessor"

static const char* CMakeListDefault =
        "cmake_minimum_required (VERSION 2.8.8)\n"
                "project (UnrealEngine)\n"
                "MACRO(HEADER_DIRECTORIES base_path return_list)\n"
                "   FILE(GLOB_RECURSE new_list ${base_path}/*.h)\n"
                "   SET(dir_list \"\")\n"
                "   FOREACH(file_path ${new_list})\n"
                "       GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)\n"
                "       SET(dir_list ${dir_list} ${dir_path})\n"
                "   ENDFOREACH()\n"
                "   LIST(REMOVE_DUPLICATES dir_list)\n"
                "   SET(${return_list} ${dir_list})\n"
                "ENDMACRO()\n"
                "HEADER_DIRECTORIES(\"<<PROJECT_ROOT>>\" ProjectHeaders)\n"
                "HEADER_DIRECTORIES(\"<<UE_ROOT>>\" UnrealHeaders)\n"
                "include_directories(${UnrealHeaders})\n"
                "include_directories(${ProjectHeaders})\n"
                "file(GLOB_RECURSE Source Source/*.cpp)\n"
                "file(GLOB_RECURSE SourceHeaders Source/*.h)\n"
                "file(GLOB_RECURSE Plugins Plugins/*.cpp)\n"
                "file(GLOB_RECURSE PluginsHeaders Plugins/*.h)\n"
                "add_executable(UnrealEngine ${Source} ${SourceHeaders} ${Plugins} ${PluginsHeaders})\n";

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
    FString OutputTemplate = this->CMakeTemplate.ToString();
    FString OutputPath = this->ProjectPath.Path + "/CMakeLists.txt";

    // Handle Project Folder
    OutputTemplate = OutputTemplate.Replace(TEXT("<<PROJECT_ROOT>>"), *this->ProjectPath.Path);

    // Handle Engine Folder
    OutputTemplate = OutputTemplate.Replace(TEXT("<<UE_ROOT>>"), *this->EnginePath.Path);

    if (FFileHelper::SaveStringToFile(OutputTemplate, *OutputPath,  FFileHelper::EEncodingOptions::Type::ForceAnsi)) {
        return true;
    }

    return false;
}