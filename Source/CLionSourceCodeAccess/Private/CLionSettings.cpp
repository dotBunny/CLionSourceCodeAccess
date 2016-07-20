// Copyright 2016 dotBunny, Inc. All Rights Reserved.

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSettings.h"

#define LOCTEXT_NAMESPACE "CLionSourceCodeAccessor"

UCLionSettings::UCLionSettings(const FObjectInitializer& ObjectInitializer)
        : Super(ObjectInitializer)
{
    // Default CLion Path
    if ( this->CLionPath.FilePath.IsEmpty())
    {
#if PLATFORM_MAC
        this->CLionPath.FilePath = TEXT("/Applications/CLion.app/Contents/MacOS/clion");
#elif PLATFORM_WINDOWS
        this->CLionPath.FilePath = TEXT("C:\\Program Files (x86)\\JetBrains\\CLion 2016.1\\bin\\clion64.exe");
#endif
    }

#if PLATFORM_MAC
    if (this->MonoPath.FilePath.IsEmpty() )
    {
     this->MonoPath.FilePath = TEXT("/Library/Frameworks/Mono.framework/Versions/Current/bin/mono");
    }
#endif

    // TODO: Other Default Values

    this->bRequestRefresh = false;

    // Check Setup
    this->CheckSetup();
}

bool UCLionSettings::CheckSetup()
{
    if (
            this->CLionPath.FilePath.IsEmpty() ||
            this->ProjectPath.Path.IsEmpty() ||
            this->ProjectFile.FilePath.IsEmpty() ||
            this->ProjectName.IsEmpty() ||
            this->UnrealBuildToolPath.FilePath.IsEmpty())
    {
        this->bSetupComplete = false;
    }
    this->bSetupComplete = true;

    return this->bSetupComplete;
}

#if WITH_EDITOR
void UCLionSettings::PreEditChange(UProperty* PropertyAboutToChange)
{
    PreviousCLionPath = this->CLionPath.FilePath;
    PreviousUnrealBuildToolPath = this->UnrealBuildToolPath.FilePath;
    PreviousProjectPath = this->ProjectPath.Path;
    PreviousProjectFile = this->ProjectFile.FilePath;
    PreviousProjectName= this->ProjectName.ToString();
    PreviousCLangPath = this->CLangPath.FilePath;
    PreviousCLangXXPath = this->CLangXXPath.FilePath;
    PreviousMonoPath = this->MonoPath.FilePath;
}

void UCLionSettings::PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent )
{
    const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    const FName MemberPropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

    // CLion Executable Path Check
    if ( MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings,CLionPath) )
    {
        this->CLionPath.FilePath = FPaths::ConvertRelativePathToFull(this->CLionPath.FilePath);
        this->CLionPath.FilePath = this->CLionPath.FilePath.Trim();
        this->CLionPath.FilePath = CLionPath.FilePath.TrimTrailing();

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

    // Mono Path
    if ( MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings,MonoPath) )
    {
        MonoPath.FilePath = FPaths::ConvertRelativePathToFull(MonoPath.FilePath);
        MonoPath.FilePath = MonoPath.FilePath.Trim();
        MonoPath.FilePath = MonoPath.FilePath.TrimTrailing();

        FText FailReason;

        if ( MonoPath.FilePath == this->PreviousMonoPath ) return;

        if (!FPaths::ValidatePath(MonoPath.FilePath, &FailReason))
        {
            FMessageDialog::Open(EAppMsgType::Ok, FailReason);
            MonoPath.FilePath = this->PreviousMonoPath;
            return;
        }

        bRequestRefresh = true;
    }

    // Post Check UBT
    if ( MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings,UnrealBuildToolPath) )
    {
        UnrealBuildToolPath.FilePath = FPaths::ConvertRelativePathToFull(UnrealBuildToolPath.FilePath);
        UnrealBuildToolPath.FilePath = UnrealBuildToolPath.FilePath.Trim();
        UnrealBuildToolPath.FilePath = UnrealBuildToolPath.FilePath.TrimTrailing();

        FText FailReason;

        if ( UnrealBuildToolPath.FilePath == this->PreviousUnrealBuildToolPath ) return;

        if (!FPaths::ValidatePath(UnrealBuildToolPath.FilePath, &FailReason))
        {
            FMessageDialog::Open(EAppMsgType::Ok, FailReason);
            UnrealBuildToolPath.FilePath = this->PreviousUnrealBuildToolPath;
            return;
        }

        bRequestRefresh = true;
    }

    // Post Check Project File
    if ( MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings,ProjectFile) )
    {
        ProjectFile.FilePath = FPaths::ConvertRelativePathToFull(ProjectFile.FilePath);
        ProjectFile.FilePath = ProjectFile.FilePath.Trim();
        ProjectFile.FilePath = ProjectFile.FilePath.TrimTrailing();

        FText FailReason;

        if ( ProjectFile.FilePath == this->PreviousProjectFile ) return;

        if (!FPaths::ValidatePath(ProjectFile.FilePath, &FailReason))
        {
            FMessageDialog::Open(EAppMsgType::Ok, FailReason);
            ProjectFile.FilePath = this->PreviousProjectFile;
            return;
        }

        bRequestRefresh = true;
    }


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

    this->CheckSetup();
}
#endif


bool UCLionSettings::IsSetup()
{
    return this->bSetupComplete;
}