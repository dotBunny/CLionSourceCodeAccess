// Copyright 2016 dotBunny, Inc. All Rights Reserved.

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSettings.h"

#define LOCTEXT_NAMESPACE "CLionSourceCodeAccessor"

UCLionSettings::UCLionSettings(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
{

	CheckSettings();
}

bool UCLionSettings::CheckSettings()
{

#if PLATFORM_WINDOWS
	if (this->CLion.FilePath.IsEmpty())
	{
		// Damn windows specific folders
		if (FPaths::FileExists(TEXT("C:\\Program Files (x86)\\JetBrains\\CLion 2016.1\\bin\\clion64.exe")))
		{
			this->CLion.FilePath = TEXT("C:\\Program Files (x86)\\JetBrains\\CLion 2016.2\\bin\\clion64.exe");
		}
		else if (FPaths::FileExists(TEXT("C:\\Program Files (x86)\\JetBrains\\CLion 2016.2\\bin\\clion.exe")))
		{
			this->CLion.FilePath = TEXT("C:\\Program Files (x86)\\JetBrains\\CLion 2016.2\\bin\\clion.exe");
		}
	}
#elif PLATFORM_MAC
    if ( this->CLion.FilePath.IsEmpty())
    {
        this->CLion.FilePath = TEXT("/Applications/CLion.app/Contents/MacOS/clion");
    }

	// Mono Mac Default Locations
    if (this->Mono.FilePath.IsEmpty() )
    {
	    if (FPaths::FileExists(TEXT("/Library/Frameworks/Mono.framework/Versions/Current/bin/mono"))) {
		    this->Mono.FilePath = TEXT("/Library/Frameworks/Mono.framework/Versions/Current/bin/mono");
	    }
    }
#else
	// Mono Linux Default Locations
	if (this->Mono.FilePath.IsEmpty() )
	{
		if (FPaths::FileExists(TEXT("/usr/bin/mono")))
		{
			this->Mono.FilePath = TEXT("/usr/bin/mono");
		}
		else if (FPaths::FileExists(TEXT("/opt/mono/bin/mono")))
		{
			this->Mono.FilePath = TEXT("/opt/mono/bin/mono");
		}
	}
#endif

	// Reset the setup complete before we check things
	this->bSetupComplete = true;

	if (this->CLion.FilePath.IsEmpty())
	{
		this->bSetupComplete = false;
	}

#if !PLATFORM_WINDOWS
	if ( this->Mono.FilePath.IsEmpty())
	{
		this->bSetupComplete = false;
	}
#endif

	return this->bSetupComplete;
}

bool UCLionSettings::IsSetup()
{
	return this->bSetupComplete;
}

#if WITH_EDITOR

void UCLionSettings::PreEditChange(UProperty *PropertyAboutToChange)
{
	PreviousCLion = this->CLion.FilePath;
	PreviousCCompiler = this->CCompiler.FilePath;
	PreviousCXXComplier = this->CXXCompiler.FilePath;
	PreviousMono = this->Mono.FilePath;
}

void UCLionSettings::PostEditChangeProperty(struct FPropertyChangedEvent &PropertyChangedEvent)
{
	const FName MemberPropertyName = (PropertyChangedEvent.Property != nullptr)
	                                 ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	// CLion Executable Path Check
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, CLion)) {
		this->CLion.FilePath = FPaths::ConvertRelativePathToFull(this->CLion.FilePath);
		this->CLion.FilePath = this->CLion.FilePath.Trim();
		this->CLion.FilePath = CLion.FilePath.TrimTrailing();

		FText FailReason;

#if PLATFORM_MAC
		if (CLion.FilePath.EndsWith(TEXT("clion.app"))) {
			CLion.FilePath = CLion.FilePath.Append(TEXT("/Contents/MacOS/clion"));
		}

		if (!CLion.FilePath.Contains(TEXT("clion.app"))) {
			FailReason = LOCTEXT("CLionSelectMacApp", "Please select the CLion app");
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			CLion.FilePath = PreviousCLion;
			return;
		}
#endif

		if (CLion.FilePath == PreviousCLion) return;

		if (!FPaths::ValidatePath(CLion.FilePath, &FailReason)) {
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			CLion.FilePath = PreviousCLion;
			return;
		}
	}


	// Mono Path
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, Mono)) {
		this->Mono.FilePath = FPaths::ConvertRelativePathToFull(this->Mono.FilePath);
		this->Mono.FilePath = this->Mono.FilePath.Trim();
		this->Mono.FilePath = this->Mono.FilePath.TrimTrailing();

		FText FailReason;

		if (this->Mono.FilePath == this->PreviousMono) return;

		if (!FPaths::ValidatePath(this->Mono.FilePath, &FailReason)) {
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			this->Mono.FilePath = this->PreviousMono;
			return;
		}
	}


	// Check C Compiler Path
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, CCompiler)) {
		this->CCompiler.FilePath = FPaths::ConvertRelativePathToFull(this->CCompiler.FilePath);
		this->CCompiler.FilePath = this->CCompiler.FilePath.Trim();
		this->CCompiler.FilePath = this->CCompiler.FilePath.TrimTrailing();

		if (this->CCompiler.FilePath == this->PreviousCCompiler) return;

		if (!FPaths::FileExists(CCompiler.FilePath)) {
			this->CCompiler.FilePath = this->PreviousCCompiler;
			return;
		}
		this->bRequireRefresh = true;
	}

	// Check C++ Compiler Path
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, CXXCompiler)) {
		this->CXXCompiler.FilePath = FPaths::ConvertRelativePathToFull(this->CXXCompiler.FilePath);
		this->CXXCompiler.FilePath = this->CXXCompiler.FilePath.Trim();
		this->CXXCompiler.FilePath = this->CXXCompiler.FilePath.TrimTrailing();

		if (this->CXXCompiler.FilePath == this->PreviousCXXComplier) return;

		if (!FPaths::FileExists(CXXCompiler.FilePath)) {
			this->CXXCompiler.FilePath = this->PreviousCXXComplier;
			return;
		}
		this->bRequireRefresh = true;
	}

	this->CheckSettings();
}

#endif


