// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
// Copyright 2016 dotBunny, Inc. All Rights Reserved.

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSourceCodeAccessor.h"

#define LOCTEXT_NAMESPACE "CLionSourceCodeAccessor"

DEFINE_LOG_CATEGORY_STATIC(LogCLionAccessor, Log, All);

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


    if (!FPaths::IsProjectFilePathSet()) {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT( "ProjectFileNotFound", "A project file was not found." ));
        return;
    }

    // Due to the currently broken production of CMakeFiles in UBT, we create a CodeLite project and convert it when
    // a viable CMakeList generation is available this will change.
    if(!this->GenerateFromCodeLiteProject())
    {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT( "ProjectGenerationFailed", "Unable to produce project files" ));
        this->Settings->bRequireRefresh = true;
    }
    else
    {
        this->Settings->bRequireRefresh = false;
    }
}

bool FCLionSourceCodeAccessor::GenerateFromCodeLiteProject()
{
    // Create our progress bar dialog.
    FScopedSlowTask ProjectGenerationTask(0, LOCTEXT("ProjectCreation", "Generating Project ..."));
    ProjectGenerationTask.MakeDialog();

    // Cache our path information
    const FString UnrealBuildToolPath = *FPaths::ConvertRelativePathToFull(*FPaths::Combine(*FPaths::EngineDir(), TEXT("Binaries"), TEXT("DotNET"), TEXT("UnrealBuildTool.exe")));
    const FString ProjectFilePath = *FPaths::ConvertRelativePathToFull(*FPaths::GetProjectFilePath());
    const FString ProjectPath = *FPaths::ConvertRelativePathToFull(*FPaths::GameDir());
    const FString ProjectName = FPaths::GetBaseFilename(ProjectFilePath, true);

    // Start our master CMakeList file
    FString OutputTemplate = TEXT("cmake_minimum_required (VERSION 2.6)\nproject (UE4)\n");

    // Pop a progress window
    FScopedSlowTask CodeLiteTask(1, LOCTEXT("ProjectCreation", "Generating CodeLite Project ..."));
    CodeLiteTask.MakeDialog();
    CodeLiteTask.EnterProgressFrame();

#if PLATFORM_WINDOWS
    const FString Parameters = FString::Printf(TEXT("\"%s -Game %s -OnlyPublic -CodeLiteFile -CurrentPlatform -NoShippingConfigs\""),
                                               *ProjectFilePath,
                                               *ProjectName);

    UE_LOG(LogCLionAccessor, Log, TEXT("%s %s"), *UnrealBuildToolPath, *Parameters);
    FProcHandle ProcessHandle = FPlatformProcess::CreateProc(*UnrealBuildToolPath, *Parameters, true, true, false, nullptr, 0, nullptr, nullptr);
#else

    const FString Parameters = FString::Printf(TEXT("%s %s -Game %s -OnlyPublic -CodeLiteFile -CurrentPlatform -NoShippingConfigs"),
                                               *UnrealBuildToolPath,
                                               *ProjectFilePath,
                                               *ProjectName);
    // Create File Process
    UE_LOG(LogCLionAccessor, Log, TEXT("%s %s"), *this->Settings->Mono.FilePath, *Parameters);
    FProcHandle ProcessHandle = FPlatformProcess::CreateProc(*this->Settings->Mono.FilePath, *Parameters, true, true, false, nullptr, 0, nullptr, nullptr);

#endif
    CodeLiteTask.EnterProgressFrame();

    // Wait for the process to finish before moving on
    FPlatformProcess::WaitForProc(ProcessHandle);

    // Setup path for Includes files
    const FString IncludeDirectoriesPath = FPaths::Combine(*ProjectPath, *FString::Printf(TEXT("%sCodeCompletionFolders.txt"), *ProjectName));
    if(!FPaths::FileExists(IncludeDirectoriesPath))
    {
        UE_LOG(LogCLionAccessor, Error, TEXT("Unable to find %s"), *IncludeDirectoriesPath);
        return false;
    }

    // Setup path for Definitions file
    const FString DefinitionsPath = FPaths::Combine(*ProjectPath, *FString::Printf(TEXT("%sCodeLitePreProcessor.txt"), *ProjectName));
    if(!FPaths::FileExists(DefinitionsPath))
    {
        UE_LOG(LogCLionAccessor, Error, TEXT("Unable to find %s"), *DefinitionsPath);
        return false;
    }

    // Setup path for our master project file
    const FString GeneratedProjectFilePath =  FPaths::Combine(*ProjectPath, *FString::Printf(TEXT("%s.workspace"), *ProjectName));
    if(!FPaths::FileExists(GeneratedProjectFilePath))
    {
        UE_LOG(LogCLionAccessor, Error, TEXT("Unable to find %s"), *GeneratedProjectFilePath);
        return false;
    }

    // Setup path for where we will output the sub CMake files
    const FString ProjectFileOutputFolder = FPaths::Combine(*ProjectPath, TEXT("Intermediate"), TEXT("ProjectFiles"));
    if(!FPaths::DirectoryExists(ProjectFileOutputFolder))
    {
        FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*ProjectFileOutputFolder);
    }

    // Gather our information on include directories
    FString IncludeDirectoriesData;
    FFileHelper::LoadFileToString(IncludeDirectoriesData, *IncludeDirectoriesPath);
    TArray<FString> IncludeDirectoriesLines;
    IncludeDirectoriesData.ParseIntoArrayLines(IncludeDirectoriesLines, true);

    FString IncludeDirectoriesContent = TEXT("set(INCLUDE_DIRECTORIES \n");
    for (FString Line : IncludeDirectoriesLines)
    {
       IncludeDirectoriesContent.Append(FString::Printf(TEXT("\t\"%s\"\n"), *Line));
    }
    IncludeDirectoriesContent.Append(TEXT(")\ninclude_directories(${INCLUDE_DIRECTORIES})\n"));

    // Output our Include Directories content and add an entry to the CMakeList
    FString IncludeDirectoriesOutputPath = FPaths::Combine(*ProjectFileOutputFolder, TEXT("IncludeDirectories.cmake"));
    if (!FFileHelper::SaveStringToFile(IncludeDirectoriesContent, *IncludeDirectoriesOutputPath,  FFileHelper::EEncodingOptions::Type::ForceAnsi)) {
        UE_LOG(LogCLionAccessor, Error, TEXT("Error writing %s"), *IncludeDirectoriesOutputPath);
        return false;
    }
    OutputTemplate.Append(FString::Printf(TEXT("include(\"%s\")\n"), *IncludeDirectoriesOutputPath));


    // Gather our information on definitions
    FString DefinitionsData;
    FFileHelper::LoadFileToString(DefinitionsData, *DefinitionsPath);
    TArray<FString> DefinitionsLines;
    DefinitionsData.ParseIntoArrayLines(DefinitionsLines, true);

    FString DefinitionsProcessed = TEXT("add_definitions(\n");
    for (FString Line : DefinitionsLines )
    {
        DefinitionsProcessed.Append(FString::Printf(TEXT("\t-D%s\n"), *Line));
    }
    DefinitionsProcessed.Append(TEXT(")\n"));

    // Output our Definitions content and add an entry to the CMakeList
    FString DefinitionsOutputPath = FPaths::Combine(*ProjectFileOutputFolder, TEXT("Definitions.cmake"));
    if(!FFileHelper::SaveStringToFile(DefinitionsProcessed, *DefinitionsOutputPath,  FFileHelper::EEncodingOptions::Type::ForceAnsi))
    {
        UE_LOG(LogCLionAccessor, Error, TEXT("Error writing %s"), *DefinitionsOutputPath);
        return false;
    }
    OutputTemplate.Append(FString::Printf(TEXT("include(\"%s\")\n"), *DefinitionsOutputPath));


    // Handle finding the project file (we'll use this to determine the subprojects)
    FXmlFile* RootGeneratedProjectFile = new FXmlFile();
    RootGeneratedProjectFile->LoadFile(*GeneratedProjectFilePath);
    const FXmlNode* RootProjectNode = RootGeneratedProjectFile->GetRootNode();
	const TArray<FXmlNode*> RootProjectNodes = RootProjectNode->GetChildrenNodes();
	TArray<FXmlNode*> ProjectNodes;

	// Iterate over nodes to come up with our project nodes
    for (FXmlNode* Node : RootProjectNodes)
    {
        if ( Node->GetTag() == TEXT("Project"))
        {
            ProjectNodes.Add(Node);
        }
    }

    // Pop a progress window
    FScopedSlowTask ProjectTasks(RootProjectNodes.Num(), LOCTEXT("ProjectCreation", "Evaluating Project Files ..."));
    ProjectTasks.MakeDialog();
    ProjectTasks.EnterProgressFrame();

    // Iterate and create projects
    FXmlFile* WorkingGeneratedProjectFile = new FXmlFile();
    FXmlNode* WorkingRootProjectNode;

    for (FXmlNode* Node : ProjectNodes) {
        // Increment Progress Bar
        ProjectTasks.EnterProgressFrame();
        FString OutputProjectTemplate = "";
        FString SubProjectName = Node->GetAttribute("Name");
        FString SubProjectFile = FPaths::Combine(*ProjectPath, *Node->GetAttribute("Path"));

        // Check the project file does exist
        if (!FPaths::FileExists(SubProjectFile)) {
            UE_LOG(LogCLionAccessor, Warning, TEXT("Cannot find %s"), *SubProjectFile);
            continue;
        }

        FString ProjectFilesProcessed;

        WorkingGeneratedProjectFile->LoadFile(*SubProjectFile);

        // This is painful as we're going to have to iterate through each node/child till we have none
        FXmlNode * CurrentNode = WorkingGeneratedProjectFile->GetRootNode();

        // Call our recursive function to delve deep and get the data we need
        FString WorkingProjectFiles = GetFilesFromCodeLiteXML(CurrentNode);

        // Add file set to the project cmake file (this is so we split things up, so CLion does't have
        // any issues with the file size of one individual file.
        OutputProjectTemplate.Append(FString::Printf(TEXT("set(%s_FILES \n%s)\n"), *SubProjectName, *WorkingProjectFiles));

        // Time to output this, determine the output path
        FString ProjectOutputPath = FPaths::Combine(*ProjectFileOutputFolder, *FString::Printf(TEXT("%s.cmake"), *SubProjectName));
        if(!FFileHelper::SaveStringToFile(OutputProjectTemplate, *ProjectOutputPath,  FFileHelper::EEncodingOptions::Type::ForceAnsi))
        {
            UE_LOG(LogCLionAccessor, Error, TEXT("Error writing %s"), *ProjectOutputPath);
            return false;
        }

        // Add Include Of Project Files
        OutputTemplate.Append(FString::Printf(TEXT("include(\"%s\")\n"), *ProjectOutputPath));
    }


    // Time to output this, determine the output path
    FString OutputPath = FPaths::Combine(*ProjectPath, TEXT("CMakeLists.txt"));

    // Handle CLang++ / CLang (If Defined)
    if ( !this->Settings->CXXCompiler.FilePath.IsEmpty() )
    {
        OutputTemplate.Append(FString::Printf(TEXT("set(CMAKE_CXX_COMPILER \"%s\")\n"), *this->Settings->CXXCompiler.FilePath));
    }
    if ( !this->Settings->CCompiler.FilePath.IsEmpty() )
    {
        OutputTemplate.Append(FString::Printf(TEXT("set(CMAKE_C_COMPILER \"%s\")\n"), *this->Settings->CCompiler.FilePath));
    }

    // Add Executable Definition To Main Template
    OutputTemplate.Append(FString::Printf(TEXT("add_executable(%sEditor ${%sEditor_FILES})\n"), *ProjectName, *ProjectName));

    // Write out the file
    if (!FFileHelper::SaveStringToFile(OutputTemplate, *OutputPath,  FFileHelper::EEncodingOptions::Type::ForceAnsi)) {
        UE_LOG(LogCLionAccessor, Error, TEXT("Error writing %s"), *OutputPath);
        return false;
    }

    return true;
}

FString FCLionSourceCodeAccessor::GetFilesFromCodeLiteXML(FXmlNode* CurrentNode) {

    FString ReturnContent = "";

    if ( CurrentNode->GetTag() == "File" ) {
        ReturnContent.Append(FString::Printf(TEXT("\t\"%s\"\n"), *CurrentNode->GetAttribute("Name")));
    }

    const TArray<FXmlNode*> childrenNodes = CurrentNode->GetChildrenNodes();
    for (FXmlNode* Node : childrenNodes)
    {
        ReturnContent += GetFilesFromCodeLiteXML(Node);
    }

    return ReturnContent;

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

    this->Settings->CheckSettings();
}

bool FCLionSourceCodeAccessor::OpenFileAtLine(const FString& FullPath, int32 LineNumber, int32 ColumnNumber)
{
    if (!this->Settings->IsSetup())
    {
        UE_LOG(LogCLionAccessor, Warning, TEXT("Please configure the CLion integration in your project settings."));
        return false;
    }


    if ( this->Settings->bRequireRefresh) {
        this->GenerateProjectFile();
    }


    const FString Path = FString::Printf(TEXT("\"%s --line %d --column %d %s\""), *FPaths::ConvertRelativePathToFull(*FPaths::GameDir()), LineNumber, ColumnNumber, *FullPath);
    if(FPlatformProcess::CreateProc(*this->Settings->CLion.FilePath, *Path, true, true, false, nullptr, 0, nullptr, nullptr).IsValid())
    {
        UE_LOG(LogCLionAccessor, Warning, TEXT("FCLionSourceCodeAccessor::OpenFileAtLine: Failed"));
        return false;
    }

    return true;
}

bool FCLionSourceCodeAccessor::OpenSolution()
{

    // TODO: Add check for CMakeProject file, if not there generate

    if ( this->Settings->bRequireRefresh) {
        this->GenerateProjectFile();
    }


    if(FPlatformProcess::CreateProc(*this->Settings->CLion.FilePath, *FPaths::ConvertRelativePathToFull(*FPaths::GameDir()), true, true, false, nullptr, 0, nullptr, nullptr).IsValid())
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

    if ( this->Settings->bRequireRefresh) {
        this->GenerateProjectFile();
    }

    for(const auto& SourcePath : AbsoluteSourcePaths)
    {
        const FString Path = FString::Printf(TEXT("\"%s\""), *SourcePath);
        FProcHandle Proc = FPlatformProcess::CreateProc(*this->Settings->CLion.FilePath, *Path, true, false, false, nullptr, 0, nullptr, nullptr);

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