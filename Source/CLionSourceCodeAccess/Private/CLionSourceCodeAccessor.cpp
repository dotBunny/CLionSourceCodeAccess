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
    FScopedSlowTask ProjectGenerationTask(21, LOCTEXT("StartCMakeListGeneration", "Starting CMakeList Generation"));
    ProjectGenerationTask.MakeDialog();
    ProjectGenerationTask.EnterProgressFrame(1, LOCTEXT("StartCMakeListGeneration", "Starting CMakeList Generation"));

    // Cache our path information
    FString UnrealBuildToolPath = *FPaths::ConvertRelativePathToFull(*FPaths::Combine(*FPaths::EngineDir(), TEXT("Binaries"), TEXT("DotNET"), TEXT("UnrealBuildTool.exe")));
    FPaths::NormalizeFilename(UnrealBuildToolPath);
    FString ProjectFilePath = *FPaths::ConvertRelativePathToFull(*FPaths::GetProjectFilePath());
    FPaths::NormalizeFilename(ProjectFilePath);
    FString ProjectPath = *FPaths::ConvertRelativePathToFull(*FPaths::GameDir());
    FPaths::NormalizeFilename(ProjectPath);
    FString ProjectName = FPaths::GetBaseFilename(ProjectFilePath, true);
    FPaths::NormalizeFilename(ProjectName);

    // Start our master CMakeList file
    FString OutputTemplate = TEXT("cmake_minimum_required (VERSION 2.6)\nproject (UE4)\n");
    OutputTemplate.Append(TEXT("set(CMAKE_CXX_STANDARD 11)\n\n"));

    // Increase our progress
    ProjectGenerationTask.EnterProgressFrame(10, LOCTEXT("GeneratingCodLiteProject", "Generating CodeLite Project"));

#if PLATFORM_WINDOWS
	const FString BuildProjectCommand = *UnrealBuildToolPath;
    const FString BuildProjectParameters = FString::Printf(TEXT("\"%s\" -Game \"%s\" -OnlyPublic -CodeLiteFile -CurrentPlatform -NoShippingConfigs"),
                                               *ProjectFilePath,
                                               *ProjectName);
#else
	const FString BuildProjectCommand = *this->Settings->Mono.FilePath;
    const FString BuildProjectParameters = FString::Printf(TEXT("\"%s\" \"%s\" -Game \"%s\" -OnlyPublic -CodeLiteFile -CurrentPlatform -NoShippingConfigs"),
                                               *UnrealBuildToolPath,
                                               *ProjectFilePath,
                                               *ProjectName);
#endif

	FProcHandle BuildProjectProcess = FPlatformProcess::CreateProc(*BuildProjectCommand, *BuildProjectParameters, true, true, false, nullptr, 0, nullptr, nullptr);
	if (!BuildProjectProcess.IsValid())
	{
		UE_LOG(LogCLionAccessor, Error, TEXT("Failure to run UBT [%s %s]."), *BuildProjectCommand, *BuildProjectParameters);
		FPlatformProcess::CloseProc(BuildProjectProcess);
		return false;
	}

    // Wait for the process to finish before moving on
    FPlatformProcess::WaitForProc(BuildProjectProcess);

    // Enter next phase of generation
    ProjectGenerationTask.EnterProgressFrame(1, LOCTEXT("CheckingFiles", "Checking Files"));

    // Setup path for Includes files
    FString IncludeDirectoriesPath = FPaths::Combine(*ProjectPath, *FString::Printf(TEXT("%sCodeCompletionFolders.txt"), *ProjectName));
    FPaths::NormalizeFilename(IncludeDirectoriesPath);
    if(!FPaths::FileExists(IncludeDirectoriesPath))
    {
        UE_LOG(LogCLionAccessor, Error, TEXT("Unable to find %s"), *IncludeDirectoriesPath);
        ProjectGenerationTask.EnterProgressFrame(9);
        return false;
    }

    // Setup path for Definitions file
    FString DefinitionsPath = FPaths::Combine(*ProjectPath, *FString::Printf(TEXT("%sCodeLitePreProcessor.txt"), *ProjectName));
    FPaths::NormalizeFilename(DefinitionsPath);
    if(!FPaths::FileExists(DefinitionsPath))
    {
        UE_LOG(LogCLionAccessor, Error, TEXT("Unable to find %s"), *DefinitionsPath);
        ProjectGenerationTask.EnterProgressFrame(9);
        return false;
    }

    // Setup path for our master project file
    FString GeneratedProjectFilePath =  FPaths::Combine(*ProjectPath, *FString::Printf(TEXT("%s.workspace"), *ProjectName));
    FPaths::NormalizeFilename(GeneratedProjectFilePath);
    if(!FPaths::FileExists(GeneratedProjectFilePath))
    {
        UE_LOG(LogCLionAccessor, Error, TEXT("Unable to find %s"), *GeneratedProjectFilePath);
        ProjectGenerationTask.EnterProgressFrame(9);
        return false;
    }

    // Setup path for where we will output the sub CMake files
    FString ProjectFileOutputFolder = FPaths::Combine(*ProjectPath, TEXT("Intermediate"), TEXT("ProjectFiles"));
    FPaths::NormalizeFilename(ProjectFileOutputFolder);
    if(!FPaths::DirectoryExists(ProjectFileOutputFolder))
    {
        FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*ProjectFileOutputFolder);
    }

    ProjectGenerationTask.EnterProgressFrame(3, LOCTEXT("CreatingHelperCMakeFiles", "Creating Helper CMake Files"));


    // Gather our information on include directories
    FString IncludeDirectoriesData;
    FFileHelper::LoadFileToString(IncludeDirectoriesData, *IncludeDirectoriesPath);
    TArray<FString> IncludeDirectoriesLines;
    IncludeDirectoriesData.ParseIntoArrayLines(IncludeDirectoriesLines, true);

    FString IncludeDirectoriesContent = TEXT("set(INCLUDE_DIRECTORIES \n");
    for (FString Line : IncludeDirectoriesLines)
    {
        FPaths::NormalizeFilename(Line);
        IncludeDirectoriesContent.Append(FString::Printf(TEXT("\t\"%s\"\n"), *Line));
    }
    IncludeDirectoriesContent.Append(TEXT(")\ninclude_directories(${INCLUDE_DIRECTORIES})\n"));

    // Output our Include Directories content and add an entry to the CMakeList
    FString IncludeDirectoriesOutputPath = FPaths::Combine(*ProjectFileOutputFolder, TEXT("IncludeDirectories.cmake"));
    FPaths::NormalizeFilename(IncludeDirectoriesOutputPath);
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


    ProjectGenerationTask.EnterProgressFrame(5, LOCTEXT("CreatingProjectCMakeFiles", "Creating Project CMake Files"));

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

	//WorkspaceConfiguration (DebugGame/Development/Shipping)

    // Iterate and create projects
    FXmlFile* WorkingGeneratedProjectFile = new FXmlFile();

	FScopedSlowTask SubProjectGenerationTask(ProjectNodes.Num(), LOCTEXT("StartSubProjects", "Generating Sub Project File"));
	SubProjectGenerationTask.MakeDialog();


    for (FXmlNode* Node : ProjectNodes) {
        // Increment Progress Bar
        FString OutputProjectTemplate = "";
        FString SubProjectName = Node->GetAttribute("Name");
        FString SubProjectFile = FPaths::Combine(*ProjectPath, *Node->GetAttribute("Path"));

	    SubProjectGenerationTask.EnterProgressFrame(1);

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
        FString WorkingProjectFiles = FCLionSourceCodeAccessor::GetAttributeByTagWithRestrictions(CurrentNode ,
                                                                                                  TEXT("File"),
                                                                                                  TEXT("Name"),
                                                                                                  this->Settings->bIncludeConfigs,
                                                                                                  this->Settings->bIncludePlugins,
                                                                                                  this->Settings->bIncludeShaders);
        TArray<FString> WorkingProjectFilesLines;
        WorkingProjectFiles.ParseIntoArrayLines(WorkingProjectFilesLines, true);

        FString WorkingProjectFilesContent = TEXT("");
        for (FString Line : WorkingProjectFilesLines)
        {
            FPaths::NormalizeFilename(Line);
            WorkingProjectFilesContent.Append(FString::Printf(TEXT("%s\n"), *Line));
        }

        // Add file set to the project cmake file (this is so we split things up, so CLion does't have
        // any issues with the file size of one individual file.
        OutputProjectTemplate.Append(FString::Printf(TEXT("set(%s_FILES \n%s)\n"), *SubProjectName, *WorkingProjectFilesContent));

        // Time to output this, determine the output path
        FString ProjectOutputPath = FPaths::Combine(*ProjectFileOutputFolder, *FString::Printf(TEXT("%s.cmake"), *SubProjectName));
        if(!FFileHelper::SaveStringToFile(OutputProjectTemplate, *ProjectOutputPath,  FFileHelper::EEncodingOptions::Type::ForceAnsi))
        {
            UE_LOG(LogCLionAccessor, Error, TEXT("Error writing %s"), *ProjectOutputPath);
            return false;
        }

        // Add Include Of Project Files
        OutputTemplate.Append(FString::Printf(TEXT("include(\"%s\")\n"), *ProjectOutputPath));

        //Get working directory and build command
        FString CustomTargets = FCLionSourceCodeAccessor::GetBuildCommands(CurrentNode, SubProjectName);
        OutputTemplate.Append(CustomTargets);
    }

    ProjectGenerationTask.EnterProgressFrame(1, LOCTEXT("CreatingCMakeListsFile", "Creating CMakeLists.txt File"));

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
    OutputTemplate.Append(FString::Printf(TEXT("\nadd_executable(%sEditorFake ${%sEditor_FILES})\n"), *ProjectName, *ProjectName));

    // Write out the file
    if (!FFileHelper::SaveStringToFile(OutputTemplate, *this->Settings->GetCMakeListPath(),  FFileHelper::EEncodingOptions::Type::ForceAnsi)) {
        UE_LOG(LogCLionAccessor, Error, TEXT("Error writing %s"), *this->Settings->GetCMakeListPath());
        return false;
    }

    return true;
}

FString FCLionSourceCodeAccessor::GetAttributeByTagWithRestrictions(FXmlNode *CurrentNode, const FString &Tag, const FString &Attribute, const bool &IncludeConfigs, const bool &IncludePlugins, const bool &IncludeShaders)
{
    FString ReturnContent = "";

    if ( CurrentNode->GetTag() == Tag ) {
        ReturnContent.Append(FString::Printf(TEXT("\t\"%s\"\n"), *CurrentNode->GetAttribute(Attribute)));
    }

    const TArray<FXmlNode*> childrenNodes = CurrentNode->GetChildrenNodes();
    for (FXmlNode* Node : childrenNodes)
    {
        //Don't get files from "Config", "Plugins", "Shaders"
        if (Node->GetTag() == TEXT("VirtualDirectory")) {
            const FString & name = Node->GetAttribute(TEXT("Name"));

			if (!IncludeConfigs && name == TEXT("Config"))
			{
				continue;
			}
	        if (!IncludePlugins && name == TEXT("Plugins"))
	        {
		        continue;
	        }
	        if (!IncludeShaders && name == TEXT("Shaders"))
	        {
		        continue;
	        }
        }

        ReturnContent += FCLionSourceCodeAccessor::GetAttributeByTagWithRestrictions(Node, Tag, Attribute, IncludeConfigs, IncludePlugins, IncludeShaders);
    }

    return ReturnContent;
}

FString FCLionSourceCodeAccessor::GetBuildCommands(FXmlNode *CurrentNode, const FString &SubprojectName)
{
    FString ReturnContent = "";
    FString MonoPath = "";

    if (CurrentNode->GetTag() != TEXT("Settings")) {
        const TArray<FXmlNode*> childrenNodes = CurrentNode->GetChildrenNodes();
        for (FXmlNode* Node : childrenNodes) {
            ReturnContent += FCLionSourceCodeAccessor::GetBuildCommands(Node, SubprojectName);
        }
    } else {
        const TArray<FXmlNode*> childrenNodes = CurrentNode->GetChildrenNodes();
        for (FXmlNode* Node : childrenNodes) {
            if (Node->GetTag() == TEXT("Configuration")) {
                ReturnContent += FCLionSourceCodeAccessor::HandleConfiguration(Node, SubprojectName, MonoPath);
            }
        }
    }

    return ReturnContent;
}

FString FCLionSourceCodeAccessor::HandleConfiguration(FXmlNode *CurrentNode, const FString &SubprojectName, FString &MonoPath) {
    FString ReturnContent = "";

    const FString & ConfigurationName = CurrentNode->GetAttribute(TEXT("Name"));

    FString WorkingDirectory = "";
    FString BuildCommand = "";
    FString CleanCommand = "";

    const TArray<FXmlNode*> childrenNodes = CurrentNode->GetChildrenNodes();
    for (FXmlNode* Node : childrenNodes) {

        if ((Node->GetTag() == TEXT("CustomBuild")) && (Node->GetAttribute(TEXT("Enabled")) == TEXT("yes"))) {

            const TArray<FXmlNode*> subchildrenNodes = Node->GetChildrenNodes();

            for (FXmlNode* subNode : subchildrenNodes) {
                if (subNode->GetTag() == TEXT("WorkingDirectory")) {
                    WorkingDirectory = subNode->GetContent();
                    FPaths::NormalizeFilename(WorkingDirectory);
                }

                if (subNode->GetTag() == TEXT("BuildCommand")) {
                    BuildCommand = subNode->GetContent();
                    FPaths::NormalizeFilename(BuildCommand);
                }

                if (subNode->GetTag() == TEXT("CleanCommand")) {
                    CleanCommand = subNode->GetContent();
                    FPaths::NormalizeFilename(CleanCommand);
                }
            }

            ReturnContent +=
                    FString::Printf(TEXT("\n# Custom target for %s project, %s configuration\n"), *SubprojectName, *ConfigurationName);
            if (MonoPath != WorkingDirectory) { // Do this to avoid duplication in CMakeLists.txt
                ReturnContent +=
                        FString::Printf(TEXT("set(MONO_ROOT_PATH \"%s\")\n") , *WorkingDirectory);
                MonoPath = WorkingDirectory;

                ReturnContent +=
                        FString::Printf(TEXT("set(BUILD cd \"${MONO_ROOT_PATH}\")\n\n"));
            }

            if (ConfigurationName == TEXT("Development")) {
                ReturnContent +=
                        FString::Printf(TEXT("add_custom_target(%s ${BUILD} && %s -game)\n") , *SubprojectName , *BuildCommand);
                ReturnContent +=
                        FString::Printf(TEXT("add_custom_target(%s-clean ${BUILD} && %s)\n\n") , *SubprojectName , *CleanCommand);
            } else {
                ReturnContent +=
                        FString::Printf(TEXT("add_custom_target(%s-Mac-%s ${BUILD} && %s -game)\n") , *SubprojectName , *ConfigurationName , *BuildCommand);
                ReturnContent +=
                        FString::Printf(TEXT("add_custom_target(%s-Mac-%s-clean ${BUILD} && %s)\n\n") , *SubprojectName , *ConfigurationName , *CleanCommand);
            }
        }
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

bool FCLionSourceCodeAccessor::OpenFileAtLine(const FString& FullPath, int32 LineNumber, int32 ColumnNumber)
{
    if (!this->Settings->IsSetup())
    {
        UE_LOG(LogCLionAccessor, Warning, TEXT("Please configure the CLion integration in your project settings."));
        return false;
    }

    if ( this->Settings->bRequireRefresh || !FPaths::FileExists(*this->Settings->GetCMakeListPath())) {
        this->GenerateProjectFile();
    }


    const FString Path = FString::Printf(TEXT("\"%s --line %d --column %d %s\""), *FPaths::ConvertRelativePathToFull(*FPaths::GameDir()), LineNumber, ColumnNumber, *FullPath);

    FProcHandle Proc = FPlatformProcess::CreateProc(*this->Settings->CLion.FilePath, *Path, true, true, false, nullptr, 0, nullptr, nullptr);
    if(!Proc.IsValid())
    {
        UE_LOG(LogCLionAccessor, Warning, TEXT("Opening file (%s) at a specific line failed."), *Path);
        FPlatformProcess::CloseProc(Proc);
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

    if ( this->Settings->bRequireRefresh || !FPaths::FileExists(*this->Settings->GetCMakeListPath())) {
        this->GenerateProjectFile();
    }

    const FString Path = FString::Printf(TEXT("\"%s\""), *FPaths::ConvertRelativePathToFull(*FPaths::GameDir()));
    if(FPlatformProcess::CreateProc(*this->Settings->CLion.FilePath, *Path, true, true, false, nullptr, 0, nullptr, nullptr).IsValid())
    {
        // This seems to always fail no matter what we do - could be the process type? Get rid of the warning for now
        //UE_LOG(LogCLionAccessor, Warning, TEXT("Opening the solution failed."));
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

    if ( this->Settings->bRequireRefresh || !FPaths::FileExists(*this->Settings->GetCMakeListPath())) {
        this->GenerateProjectFile();
    }

    for(const auto& SourcePath : AbsoluteSourcePaths)
    {
        const FString Path = FString::Printf(TEXT("\"%s\""), *SourcePath);
        FProcHandle Proc = FPlatformProcess::CreateProc(*this->Settings->CLion.FilePath, *Path, true, false, false, nullptr, 0, nullptr, nullptr);

        if(!Proc.IsValid())
        {
            UE_LOG(LogCLionAccessor, Warning, TEXT("Opening the source file (%s) failed."), *Path);
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

#undef LOCTEXT_NAMESPACE
