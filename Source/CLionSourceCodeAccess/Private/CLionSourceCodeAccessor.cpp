// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
// Copyright 2016 dotBunny, Inc. All Rights Reserved.

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSourceCodeAccessor.h"

#define LOCTEXT_NAMESPACE "CLionSourceCodeAccessor"

DEFINE_LOG_CATEGORY_STATIC(LogCLionAccessor, Log, All);

static const char* CMakeTemplate =
        // Establish the minimum version of CMake
        "cmake_minimum_required (VERSION 3.3)\n"

        // Name the project the normal generated project name
        "project (UE4)\n"
        "\n"
        "set(INCLUDE_FOLDERS\n"
        "<<INCLUDE_FOLDERS>>)\n"
        "\n"
        "add_definitions(\n"
        "<<DEFINITIONS>>)\n"
        "\n"
        "include_directories(${INCLUDE_FOLDERS})\n"
        "\n"
        // Handle Our Project Files Specifically
        "file(GLOB_RECURSE Source Source<<DIRECTORY_SEPARATOR>>*.cpp)\n"
        "file(GLOB_RECURSE SourceHeaders Source<<DIRECTORY_SEPARATOR>>*.h)\n"
        "file(GLOB_RECURSE Plugins Plugins<<DIRECTORY_SEPARATOR>>*.cpp)\n"
        "file(GLOB_RECURSE PluginsHeaders Plugins<<DIRECTORY_SEPARATOR>>*.h)\n"
        "file(GLOB_RECURSE Intermediate Intermediate<<DIRECTORY_SEPARATOR>>*.cpp)\n"
        "file(GLOB_RECURSE IntermediateHeaders Intermediate<<DIRECTORY_SEPARATOR>>*.h)\n"
        "\n"
        "add_executable(UnrealEngine ${Source} ${SourceHeaders} ${Intermediate} ${IntermediateHeaders} ${Plugins} ${PluginsHeaders})\n";



bool FCLionSourceCodeAccessor::AddSourceFiles(const TArray<FString>& AbsoluteSourcePaths, const TArray<FString>& AvailableModules)
{
    // There is no need to add the files to the make file because it already has wildcard searches of the necessary project folders
    return true;
}

bool FCLionSourceCodeAccessor::CanAccessSourceCode() const
{
    return this->Settings->IsSetup();
}

void FCLionSourceCodeAccessor::GenerateProjectFile()
{

    if ( !this->Settings->IsSetup() ) {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT( "SetupNotComplete", "The CLion plugin settings have not been completed." ));
        return;
    }

    // Due to the currently broken production of CMakeFiles in UBT, we create a CodeLite project and convert it when
    // a viable CMakeList generation is available this will change.
    this->GenerateFromCodeLiteProject();
}


void FCLionSourceCodeAccessor::GenerateFromCodeLiteProject()
{
#if PLATFORM_WINDOWS
    const FString DirectorySeperator = TEXT("\\");
#else
    const FString DirectorySeperator = TEXT("/");
#endif


    // Pop a progress window
    FScopedSlowTask SlowTask(0, LOCTEXT("ProjectCreation", "Creating Project..."));
    SlowTask.MakeDialog();

#if PLATFORM_WINDOWS
    const FString Parameters = FString::Printf(TEXT("\"%s -Game %s -OnlyPublic -CodeLiteFile -CurrentPlatform -NoShippingConfigs\""),
                                               *this->Settings->ProjectFile.FilePath,
                                               *this->Settings->ProjectName.ToString());

    UE_LOG(LogCLionAccessor, Log, TEXT("%s %s"), *this->Settings->UnrealBuildToolPath.FilePath, *Parameters);
    FProcHandle ProcessHandle = FPlatformProcess::CreateProc(*this->Settings->UnrealBuildToolPath.FilePath, *Parameters, true, true, false, nullptr, 0, *this->Settings->ProjectPath.Path, nullptr);
#else

    const FString Parameters = FString::Printf(TEXT("%s %s -Game %s -OnlyPublic -CodeLiteFile -CurrentPlatform -NoShippingConfigs"),
                                               *this->Settings->UnrealBuildToolPath.FilePath,
                                               *this->Settings->ProjectFile.FilePath,
                                               *this->Settings->ProjectName.ToString());
    // Create File Process
    UE_LOG(LogCLionAccessor, Log, TEXT("%s %s"), *this->Settings->MonoPath.FilePath, *Parameters);
    FProcHandle ProcessHandle = FPlatformProcess::CreateProc(*this->Settings->MonoPath.FilePath, *Parameters, true, true, false, nullptr, 0, nullptr, nullptr);

#endif

    // Wait for the process to finish before moving on
    FPlatformProcess::WaitForProc(ProcessHandle);

    // Setup paths to our puppies
#if PLATFORM_WINDOWS
    const FString CodeCompletionFoldersPath = FString::Printf(TEXT("%s\\%s%s"), *this->Settings->ProjectPath.Path, *this->Settings->ProjectName.ToString(), TEXT("CodeCompletionFolders.txt"));
    const FString DefinesPath = FString::Printf(TEXT("%s\\%s%s"), *this->Settings->ProjectPath.Path, *this->Settings->ProjectName.ToString(), TEXT("CodeLitePreProcessor.txt"));
#else
    const FString CodeCompletionFoldersPath = FString::Printf(TEXT("%s/%s%s"), *this->Settings->ProjectPath.Path, *this->Settings->ProjectName.ToString(), TEXT("CodeCompletionFolders.txt"));
    const FString DefinesPath = FString::Printf(TEXT("%s/%s%s"), *this->Settings->ProjectPath.Path, *this->Settings->ProjectName.ToString(), TEXT("CodeLitePreProcessor.txt"));
#endif

    // Check files exist

    if(!FPaths::ValidatePath(CodeCompletionFoldersPath))
    {
        UE_LOG(LogCLionAccessor, Log, TEXT("Path Invalidated - %s"), *CodeCompletionFoldersPath);
        return;
    }

    if(!FPaths::ValidatePath(DefinesPath))
    {
        UE_LOG(LogCLionAccessor, Log, TEXT("Path Invalidated - %s"), *DefinesPath);
        return;
    }

    FString CodeCompletionData = TEXT("");
    FString DefinesData = TEXT("");

    FFileHelper::LoadFileToString(CodeCompletionData, *CodeCompletionFoldersPath);
    FFileHelper::LoadFileToString(DefinesData, *DefinesPath);

    TArray<FString> CodeCompletionLines;
    CodeCompletionData.ParseIntoArrayLines(CodeCompletionLines, true);

    TArray<FString> DefinesLines;
    DefinesData.ParseIntoArrayLines(DefinesLines, true);

    FString CodeCompletionProcessed;
    for (FString Line : CodeCompletionLines)
    {
       CodeCompletionProcessed += "\t\"" + Line + "\"\n";
    }

    FString DefinitionsProcessed;
    for (FString Line : DefinesLines )
    {
        DefinitionsProcessed += "\t-D" + Line + "\n";
    }


    // Output Time
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

    FString OutputTemplate = FString(CMakeTemplate);
    FString OutputPath = this->Settings->ProjectPath.Path + DirectorySeparator + "CMakeLists.txt";

    // Handle CLang++ / CLang (If Defined)
    if ( !this->Settings->CLangXXPath.FilePath.IsEmpty() )
    {
        FString CLangXXSetting = TEXT("set(CMAKE_CXX_COMPILER \"") + this->Settings->CLangXXPath.FilePath + "\")\n";
        OutputTemplate = OutputTemplate.Append(CLangXXSetting);
    }
    if ( !this->Settings->CLangPath.FilePath.IsEmpty() )
    {
        FString CLangSetting = TEXT("set(CMAKE_C_COMPILER \"") + this->Settings->CLangPath.FilePath + "\")\n";
        OutputTemplate = OutputTemplate.Append(CLangSetting);
    }

    // Platform Specific
    OutputTemplate = OutputTemplate.Replace(TEXT("<<DIRECTORY_SEPARATOR>>"), *DirectorySeparator);
    OutputTemplate = OutputTemplate.Replace(TEXT("<<PLATFORM>>"),*PlatformName);
    OutputTemplate = OutputTemplate.Replace(TEXT("<<PLATFORM_CODE>>"),*PlatformCode);

    // Code Completion
    OutputTemplate = OutputTemplate.Replace(TEXT("<<INCLUDE_FOLDERS>>"),*CodeCompletionProcessed);

    // Definitions
    OutputTemplate = OutputTemplate.Replace(TEXT("<<DEFINITIONS>>"),*DefinitionsProcessed);



    // Write out the file
    if (FFileHelper::SaveStringToFile(OutputTemplate, *OutputPath,  FFileHelper::EEncodingOptions::Type::ForceAnsi)) {
        //return true;
    }
}

FText FCLionSourceCodeAccessor::GetDescriptionText() const
{
	return LOCTEXT("CLionDisplayDesc", "Open source code files in CLion");
}

FName FCLionSourceCodeAccessor::GetFName() const
{
	return FName("CLionSourceCodeAccessor");
}

FText FCLionSourceCodeAccessor::GetNameText() const
{
	return LOCTEXT("CLionDisplayName", "CLion");
}

void FCLionSourceCodeAccessor::Shutdown()
{
    this->Settings = nullptr;
}

void FCLionSourceCodeAccessor::Startup()
{
    // Get reference to our settings object
    this->Settings = GetMutableDefault<UCLionSettings>();
}

bool FCLionSourceCodeAccessor::OpenFileAtLine(const FString& FullPath, int32 LineNumber, int32 ColumnNumber)
{
    if (!this->Settings->IsSetup())
    {
        UE_LOG(LogCLionAccessor, Warning, TEXT("Please configure the CLion integration in your project settings."));
        return false;
    }

    const FString Path = FString::Printf(TEXT("\"%s --line %d\""), *FullPath, LineNumber);
    if(FPlatformProcess::CreateProc(*this->Settings->CLionPath.FilePath, *Path, true, true, false, nullptr, 0, nullptr, nullptr).IsValid())
    {
        UE_LOG(LogCLionAccessor, Warning, TEXT("FCLionSourceCodeAccessor::OpenFileAtLine: Failed"));
        return false;
    }

    return true;
}

bool FCLionSourceCodeAccessor::OpenSolution()
{

    // TODO: Add check for CMakeProject file, if not there generate

    if(FPlatformProcess::CreateProc(*this->Settings->CLionPath.FilePath, *this->Settings->ProjectPath.Path, true, true, false, nullptr, 0, nullptr, nullptr).IsValid())
    {
        UE_LOG(LogCLionAccessor, Warning, TEXT("FCLionSourceCodeAccessor::OpenSolution: Failed"));
        return false;
    }
    return true;
}

bool FCLionSourceCodeAccessor::OpenSourceFiles(const TArray<FString>& AbsoluteSourcePaths)
{
    if (!this->Settings->IsSetup())
    {
        UE_LOG(LogCLionAccessor, Warning, TEXT("Please configure the CLion integration in your project settings."));
        return false;
    }

    for(const auto& SourcePath : AbsoluteSourcePaths)
    {
        const FString Path = FString::Printf(TEXT("\"%s\""), *SourcePath);
        FProcHandle Proc = FPlatformProcess::CreateProc(*this->Settings->CLionPath.FilePath, *Path, true, false, false, nullptr, 0, nullptr, nullptr);

        if(Proc.IsValid())
        {
            UE_LOG(LogCLionAccessor, Warning, TEXT("FCLionSourceCodeAccessor::OpenSourceFiles: Failed"));
            FPlatformProcess::CloseProc(Proc);
            return true;
        }
    }
    return false;
}

bool FCLionSourceCodeAccessor::SaveAllOpenDocuments() const
{
    // TODO: Implement saving remotely?
    return true;
}

void FCLionSourceCodeAccessor::Tick(const float DeltaTime)
{

}


#undef LOCTEXT_NAMESPACE