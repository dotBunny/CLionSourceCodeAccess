// Copyright 2017 dotBunny Inc. All Rights Reserved.

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSettings.h"

#define LOCTEXT_NAMESPACE "CLionSourceCodeAccessor"

UCLionSettings::UCLionSettings(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
{

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
	if (this->CLion.FilePath.IsEmpty())
	{
		if (FPaths::FileExists(TEXT("/Applications/CLion.app/Contents/MacOS/clion")))
		{
			this->CLion.FilePath = TEXT("/Applications/CLion.app/Contents/MacOS/clion");
		}
	}
	if (this->Mono.FilePath.IsEmpty())
	{
		if (FPaths::FileExists(TEXT("/Library/Frameworks/Mono.framework/Versions/Current/bin/mono")))
		{
			this->Mono.FilePath = TEXT("/Library/Frameworks/Mono.framework/Versions/Current/bin/mono");
		}
	}
	if (this->CCompiler.FilePath.IsEmpty())
	{
		if (FPaths::FileExists(TEXT("/usr/bin/clang")))
		{
			this->CCompiler.FilePath = TEXT("/usr/bin/clang");
		}
	}
	if (this->CXXCompiler.FilePath.IsEmpty())
	{
		if (FPaths::FileExists(TEXT("/usr/bin/clang++")))
		{
			this->CXXCompiler.FilePath = TEXT("/usr/bin/clang++");
		}
	}
#else
	if ( this->CLion.FilePath.IsEmpty())
	{
		if(FPaths::FileExists(TEXT("/opt/clion/bin/clion.sh")))
		{
			this->CLion.FilePath = TEXT("/opt/clion/bin/clion.sh");
		}
	}
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
	if (this->CCompiler.FilePath.IsEmpty())
	{
		if(FPaths::FileExists(TEXT("/usr/bin/clang")))
		{
			this->CCompiler.FilePath = TEXT("/usr/bin/clang");
		}
	}
	if (this->CXXCompiler.FilePath.IsEmpty())
	{
		if(FPaths::FileExists(TEXT("/usr/bin/clang++")))
		{
			this->CXXCompiler.FilePath = TEXT("/usr/bin/clang++");
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
	if (this->Mono.FilePath.IsEmpty())
	{
		this->bSetupComplete = false;
	}
#endif


	// Update CMakeList path
	this->CachedCMakeListPath = FPaths::Combine(*FPaths::ConvertRelativePathToFull(*FPaths::GameDir()),
	                                            TEXT("CMakeLists.txt"));

	return this->bSetupComplete;
}

FString UCLionSettings::GetCMakeListPath()
{
	return this->CachedCMakeListPath;
}

bool UCLionSettings::IsSetup()
{
	return this->bSetupComplete;
}


#if WITH_EDITOR

void UCLionSettings::PreEditChange(UProperty* PropertyAboutToChange)
{

	Super::PreEditChange(PropertyAboutToChange);

	// Cache our previous values
	this->PreviousCCompiler = this->CCompiler.FilePath;
	this->PreviousCLion = this->CLion.FilePath;
	this->PreviousCXXCompiler = this->CXXCompiler.FilePath;
#if !PLATFORM_WINDOWS
	this->PreviousMono = this->Mono.FilePath;
#endif

}

void UCLionSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName MemberPropertyName = (PropertyChangedEvent.Property != nullptr)
	                                 ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	// CLion Executable Path Check
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, CLion))
	{
		this->CLion.FilePath = FPaths::ConvertRelativePathToFull(this->CLion.FilePath);

		FText FailReason;

#if PLATFORM_MAC
		if (this->CLion.FilePath.EndsWith(TEXT("clion.app")))
		{
			this->CLion.FilePath = this->CLion.FilePath.Append(TEXT("/Contents/MacOS/clion"));
		}

		if (!this->CLion.FilePath.Contains(TEXT("clion.app")))
		{
			FailReason = LOCTEXT("CLionSelectMacApp", "Please select the CLion app");
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			this->CLion.FilePath = this->PreviousCLion;
			return;
		}
#endif

		if (this->CLion.FilePath == this->PreviousCLion)
		{
			return;
		}

		if (!FPaths::ValidatePath(this->CLion.FilePath, &FailReason))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			this->CLion.FilePath = this->PreviousCLion;
			return;
		}
	}

#if !PLATFORM_WINDOWS
	// Mono Path
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, Mono))
	{
		this->Mono.FilePath = FPaths::ConvertRelativePathToFull(this->Mono.FilePath);

		FText FailReason;

		if (this->Mono.FilePath == this->PreviousMono)
		{
			return;
		}

		if (!FPaths::ValidatePath(this->Mono.FilePath, &FailReason))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			this->Mono.FilePath = this->PreviousMono;
			return;
		}
	}
#endif


	// Check C Compiler Path
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, CCompiler))
	{
		// Bail out if you've wiped it out
		if (this->CCompiler.FilePath.IsEmpty())
		{
			this->bRequireRefresh = true;
			return;
		}

		this->CCompiler.FilePath = FPaths::ConvertRelativePathToFull(this->CCompiler.FilePath);

		if (this->CCompiler.FilePath == this->PreviousCCompiler)
		{
			return;
		}

		if (!FPaths::FileExists(this->CCompiler.FilePath))
		{
			this->CCompiler.FilePath = this->PreviousCCompiler;
			return;
		}
		this->bRequireRefresh = true;
	}

	// Check C++ Compiler Path
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, CXXCompiler))
	{
		if (this->CXXCompiler.FilePath.IsEmpty())
		{
			this->bRequireRefresh = true;
			return;
		}

		this->CXXCompiler.FilePath = FPaths::ConvertRelativePathToFull(this->CXXCompiler.FilePath);

		if (this->CXXCompiler.FilePath == this->PreviousCXXCompiler)
		{
			return;
		}

		if (!FPaths::FileExists(CXXCompiler.FilePath))
		{
			this->CXXCompiler.FilePath = this->PreviousCXXCompiler;
			return;
		}
		this->bRequireRefresh = true;
	}

	this->CheckSettings();
}

#endif


