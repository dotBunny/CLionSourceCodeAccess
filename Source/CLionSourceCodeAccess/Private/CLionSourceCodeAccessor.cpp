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
    this->Settings->OutputCMakeList();
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
    // Write out the CMake file for good measure
    this->Settings->OutputCMakeList();

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