// Copyright 2017 dotBunny Inc. All Rights Reserved.

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSettings.h"

#if PLATFORM_WINDOWS
#include "AllowWindowsPlatformTypes.h"
#endif

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
		// search from JetBrainsToolbox folder
		FString ToolboxBinPath;

		if (FWindowsPlatformMisc::QueryRegKey(HKEY_CURRENT_USER, TEXT("Software\\JetBrains s.r.o.\\JetBrainsToolbox\\"), TEXT(""), ToolboxBinPath)) {
			FPaths::NormalizeDirectoryName(ToolboxBinPath);
			FString PatternString(TEXT("(.*)/bin"));
			FRegexPattern Pattern(PatternString);
			FRegexMatcher Matcher(Pattern, ToolboxBinPath);
			if (Matcher.FindNext())
			{
				FString ToolboxPath = Matcher.GetCaptureGroup(1);

				FString SettingJsonPath = FPaths::Combine(ToolboxPath, FString(".settings.json"));
				if (FPaths::FileExists(SettingJsonPath))
				{
					FString JsonStr;
					FFileHelper::LoadFileToString(JsonStr, *SettingJsonPath);
					TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonStr);
					TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
					if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
					{
						FString InstallLocation;
						if (JsonObject->TryGetStringField(TEXT("install_location"), InstallLocation))
						{
							if (!InstallLocation.IsEmpty())
							{
								ToolboxPath = InstallLocation;
							}
						}
					}
				}

				FString CLionHome = FPaths::Combine(ToolboxPath, FString("apps"), FString("CLion"));
				if (FPaths::DirectoryExists(CLionHome))
				{
					TArray<FString> IDEPaths;
					IFileManager::Get().FindFilesRecursive(IDEPaths, *CLionHome, TEXT("clion64.exe"), true, false);
					if (IDEPaths.Num() > 0)
					{
						this->CLion.FilePath = IDEPaths[0];
					}
				}
			}
		}
	}

	if (this->CLion.FilePath.IsEmpty())
	{
		// search from ProgID
		FString OpenCommand;

		if (!FWindowsPlatformMisc::QueryRegKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\Classes\\Applications\\clion64.exe\\shell\\open\\command\\"), TEXT(""), OpenCommand)) {
			FWindowsPlatformMisc::QueryRegKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Classes\\Applications\\clion64.exe\\shell\\open\\command\\"), TEXT(""), OpenCommand);
		}

		FString PatternString(TEXT("\"(.*)\" \".*\""));
		FRegexPattern Pattern(PatternString);
		FRegexMatcher Matcher(Pattern, OpenCommand);
		if (Matcher.FindNext())
		{
			FString IDEPath = Matcher.GetCaptureGroup(1);
			if (FPaths::FileExists(IDEPath))
			{
				this->CLion.FilePath = IDEPath;
			}
		}
	}
#elif PLATFORM_MAC
	if (this->CLion.FilePath.IsEmpty())
	{
		NSURL* AppURL = [[NSWorkspace sharedWorkspace] URLForApplicationWithBundleIdentifier:@"com.jetbrains.CLion-EAP"];
		if (AppURL != nullptr)
		{
			this->CLion.FilePath = FString([AppURL path]);
		}
	}
	if (this->CLion.FilePath.IsEmpty())
	{
		NSURL* AppURL = [[NSWorkspace sharedWorkspace] URLForApplicationWithBundleIdentifier:@"com.jetbrains.CLion"];
		if (AppURL != nullptr)
		{
			this->CLion.FilePath = FString([AppURL path]);
		}
	}
	if (this->Mono.FilePath.IsEmpty())
	{
		FString MonoPath = FPaths::Combine(*FPaths::RootDir(), TEXT("Engine"), TEXT("Binaries"), TEXT("ThirdParty"), TEXT("Mono"), TEXT("Mac"), TEXT("bin"), TEXT("mono"));
		if (FPaths::FileExists(MonoPath))
		{
			this->Mono.FilePath = MonoPath;
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
	this->CachedCMakeListPath = FPaths::Combine(*FPaths::ConvertRelativePathToFull(*FPaths::ProjectDir()),
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
		if (this->CLion.FilePath.IsEmpty())
		{
			this->bSetupComplete = false;
			return;
		}

		this->CLion.FilePath = FPaths::ConvertRelativePathToFull(this->CLion.FilePath);

		FText FailReason;

#if PLATFORM_MAC
		NSString *AppPath = [NSString stringWithUTF8String: TCHAR_TO_UTF8(*this->CLion.FilePath)];
		NSBundle *Bundle = [NSBundle bundleWithPath: AppPath];
		FString BundleId = FString([Bundle bundleIdentifier]);

		if (!BundleId.StartsWith(TEXT("com.jetbrains.CLion"), ESearchCase::CaseSensitive))
		{
			FailReason = LOCTEXT("CLionSelectMacApp", "Please select the CLion app");
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			this->CLion.FilePath = this->PreviousCLion;
			return;
		}
		this->CLion.FilePath = FString([Bundle executablePath]);
#endif

		if (this->CLion.FilePath == this->PreviousCLion)
		{
			return;
		}

		if (!FPaths::ValidatePath(this->CLion.FilePath, &FailReason))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			this->CLion.FilePath = this->PreviousCLion;
			if (this->CLion.FilePath.IsEmpty())
			{
				this->bSetupComplete = false;
			}
			return;
		}
	}

#if !PLATFORM_WINDOWS
	// Mono Path
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCLionSettings, Mono))
	{
		if (this->Mono.FilePath.IsEmpty())
		{
			this->bSetupComplete = false;
			return;
		}

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


