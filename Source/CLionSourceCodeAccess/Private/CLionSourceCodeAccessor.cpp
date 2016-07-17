// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
// Copyright 2016 dotBunny, Inc. All Rights Reserved.

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSourceCodeAccessor.h"
#include "ISourceCodeAccessModule.h"
#include "DesktopPlatformModule.h"


#define LOCTEXT_NAMESPACE "CLionSourceCodeAccessor"

DEFINE_LOG_CATEGORY_STATIC(LogCLionAccessor, Log, All);

bool FCLionSourceCodeAccessor::CanAccessSourceCode() const
{
	return FPaths::FileExists(*this->CachedCLionPath);
}

void FCLionSourceCodeAccessor::GenerateProjectFile()
{
	// Handle project checkout

}

FString FCLionSourceCodeAccessor::GetCLionExecutable() const
{
	// TODO: Make executable path automatic or atleast settable in settings?
	// TODO: Need to handle platforms
#if PLATFORM_MAC
	this->CachedCLionPath = TEXT("/Applications/CLion.app/Contents/MacOS/clion");
#elif PLATFORM_WINDOWS
    this->CachedCLionPath = TEXT("c:\\Program Files\\something.exe");
#else
    this->CachedCLionPath = nullptr;
#endif
	return this->CachedCLionPath;
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

FString FCLionSourceCodeAccessor::GetSolutionPath() const
{
	if(IsInGameThread())
	{
		FString SolutionPath;
		if(FDesktopPlatformModule::Get()->GetSolutionPath(SolutionPath))
		{
			this->CachedSolutionPath = FPaths::ConvertRelativePathToFull(SolutionPath);
		}
	}
	return this->CachedSolutionPath;
}

bool FCLionSourceCodeAccessor::IsIDERunning()
{
	return false;
}


void FCLionSourceCodeAccessor::OpenCLion()
{

}

void FCLionSourceCodeAccessor::Shutdown()
{

}
void FCLionSourceCodeAccessor::Startup()
{
    UE_LOG(LogCLionAccessor, Warning, TEXT("CLion: STARTUP"));

	// Cache Paths
	this->GetSolutionPath();
	this->GetCLionExecutable();
}





//////----------------------



bool FCLionSourceCodeAccessor::OpenSolution()
{
    UE_LOG(LogCLionAccessor, Warning,  TEXT("CLion: OPEN SOLUTION"));
    // Write out solution
    // Open makelistfiel
	return true;
}

bool FCLionSourceCodeAccessor::OpenFileAtLine(const FString& FullPath, int32 LineNumber, int32 ColumnNumber)
{
    UE_LOG(LogCLionAccessor, Warning, TEXT("CLion: OPEN FILE AT"));
    if (!this->CanAccessSourceCode())
    {
        UE_LOG(LogCLionAccessor, Warning, TEXT("FCLionSourceCodeAccessor::OpenFileAtLine: Cannot find CLion binary"));
        return false;
    }

    const FString Path = FString::Printf(TEXT("\"%s --line %d\""), *FullPath, LineNumber);

    if(FPlatformProcess::CreateProc(*this->CachedCLionPath, *Path, true, true, false, nullptr, 0, nullptr, nullptr).IsValid())
    {
        UE_LOG(LogCLionAccessor, Warning, TEXT("FCLionSourceCodeAccessor::OpenFileAtLine: Failed"));
    }

    UE_LOG(LogCLionAccessor, Warning, TEXT("FCLionSourceCodeAccessor::OpenFileAtLine: %s %d"), *FullPath, LineNumber);

    return true;
}

bool FCLionSourceCodeAccessor::OpenSourceFiles(const TArray<FString>& AbsoluteSourcePaths)
{
    UE_LOG(LogCLionAccessor, Warning, TEXT("CLion: OPEN SOURCE FILES"));
    if (!this->CanAccessSourceCode())
    {
        UE_LOG(LogCLionAccessor, Warning, TEXT("FCLionSourceCodeAccessor::OpenSourceFiles: Cannot find CLion binary"));
        return false;
    }

    for(const auto& SourcePath : AbsoluteSourcePaths)
    {
        const FString Path = FString::Printf(TEXT("\"%s\""), *SourcePath);

        FProcHandle Proc = FPlatformProcess::CreateProc(*this->CachedCLionPath, *Path, true, false, false, nullptr, 0, nullptr, nullptr);

        if(Proc.IsValid())
        {
            UE_LOG(LogCLionAccessor, Warning, TEXT("FCLionSourceCodeAccessor::OpenSourceFiles: %s"), *Path);
            FPlatformProcess::CloseProc(Proc);
            return true;
        }
    }
    return false;
}

bool FCLionSourceCodeAccessor::AddSourceFiles(const TArray<FString>& AbsoluteSourcePaths, const TArray<FString>& AvailableModules)
{
    UE_LOG(LogCLionAccessor, Warning, TEXT("CLion: ADD SOURCE FILES"));
    // Add to CMakeLists.txt
	return true;
}

bool FCLionSourceCodeAccessor::SaveAllOpenDocuments() const
{
    UE_LOG(LogCLionAccessor, Warning, TEXT("CLion: SAVE ALL"));
    // Save All ?
    //UE_LOG(LogCLionAccessor, Error, TEXT("%s"), *FString([ExecutionError description]));
    return true;
}

void FCLionSourceCodeAccessor::Tick(const float DeltaTime)
{

}




#undef LOCTEXT_NAMESPACE