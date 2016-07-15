// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
// Copyright 2016 dotBunny, Inc. All Rights Reserved.

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSourceCodeAccessor.h"
#include "DesktopPlatformModule.h"

#define LOCTEXT_NAMESPACE "CLionSourceCodeAccessor"

DEFINE_LOG_CATEGORY_STATIC(LogCLionAccessor, Log, All);


bool FCLionSourceCodeAccessor::CanAccessSourceCode() const
{
	return IFileManager::Get().DirectoryExists(*FPlatformMisc::GetXcodePath());
}

FName FCLionSourceCodeAccessor::GetFName() const
{
	return FName("CLionSourceCodeAccessor");
}

FText FCLionSourceCodeAccessor::GetNameText() const
{
	return LOCTEXT("CLionDisplayName", "CLion");
}

FText FCLionSourceCodeAccessor::GetDescriptionText() const
{
	return LOCTEXT("CLionDisplayDesc", "Open source code files in CLion");
}




//////----------------------



bool FCLionSourceCodeAccessor::OpenSolution()
{
    // Write out solution
    // Open makelistfiel
	return false;
}

bool FCLionSourceCodeAccessor::OpenFileAtLine(const FString& FullPath, int32 LineNumber, int32 ColumnNumber)
{

    return false;
}

bool FCLionSourceCodeAccessor::OpenSourceFiles(const TArray<FString>& AbsoluteSourcePaths)
{
    // Open with CLion
    return false;
}

bool FCLionSourceCodeAccessor::AddSourceFiles(const TArray<FString>& AbsoluteSourcePaths, const TArray<FString>& AvailableModules)
{
    // Add to CMakeLists.txt
	return false;
}

bool FCLionSourceCodeAccessor::SaveAllOpenDocuments() const
{
    // Save All ?
    //UE_LOG(LogCLionAccessor, Error, TEXT("%s"), *FString([ExecutionError description]));
    return false;
}

void FCLionSourceCodeAccessor::Tick(const float DeltaTime)
{
}


void FCLionSourceCodeAccessor::GenerateProjectFile()
{
    // Handle project checkout

}

void FCLionSourceCodeAccessor::OpenCLion()
{

}
#undef LOCTEXT_NAMESPACE