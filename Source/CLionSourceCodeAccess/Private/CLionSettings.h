// Copyright 2016 dotBunny, Inc. All Rights Reserved.
#pragma once

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSettings.generated.h"

UCLASS(config = EditorUserSettings, defaultconfig)
class UCLionSettings : public UObject
{
    GENERATED_UCLASS_BODY()

public:

    /**
     * Path to CLion executable, used when needing to launch CLion
     */
    UPROPERTY(Config, EditAnywhere, Category="CLion", Meta=(DisplayName="Path To CLion Executable"))
    FFilePath CLionPath;


    /**
     * Path to the Mono executable
     */
    UPROPERTY(Config, EditAnywhere, Category="Unreal", Meta=(DisplayName="Path To Mono Executable"))
    FFilePath MonoPath;


    // TODO: This should eventually not be needed, Unreal should provide this right?
    /**
     * Path to UBT executable, used when needing to build the project files.
     */
    UPROPERTY(Config, EditAnywhere, Category="Unreal", Meta=(DisplayName="Unreal Build Tool Path"))
    FFilePath UnrealBuildToolPath;

    // TODO: Can this be provided by Unreal as well?
    /**
     * Path to the root of the game project, typically where the .uproject file is located.
     */
    UPROPERTY(Config, EditAnywhere, Category="Project", Meta=(DisplayName="Project Root Folder"))
    FDirectoryPath ProjectPath;

    // TODO: Can this be provided by Unreal as well?
    /**
     * Path to the game project file, the .uproject file.
     */
    UPROPERTY(Config, EditAnywhere, Category="Project", Meta=(DisplayName="Path To Project File"))
    FFilePath ProjectFile;

    // TODO: Unreal really should be able to provide this
    /**
     * The projects name.
     */
    UPROPERTY(Config, EditAnywhere, Category="Project", Meta=(DisplayName="Project Name"))
    FText ProjectName;

    /**
     * (optional) Path to CLang++ to be used in the CMakeList files as the compiler for C++
     */
    UPROPERTY(Config, EditAnywhere, Category="CMake", Meta=(DisplayName="Path To CLang++ Executable"))
    FFilePath CLangXXPath;

    /**
     * (optional) Path to CLang to be used in the CMakeList files as the compiler for C
     */
    UPROPERTY(Config, EditAnywhere, Category="CMake", Meta=(DisplayName="Path To CLang Executable"))
    FFilePath CLangPath;

protected:
#if WITH_EDITOR
    virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent ) override;
	virtual void PreEditChange(UProperty* PropertyAboutToChange) override;
#endif

private:

    FString PreviousCLionPath;
    FString PreviousUnrealBuildToolPath;
    FString PreviousProjectPath;
    FString PreviousProjectFile;
    FString PreviousProjectName;
    FString PreviousCLangPath;
    FString PreviousCLangXXPath;


    FString PreviousMonoPath;


    bool CheckSetup();


    bool bSetupComplete = false;

public:
    bool bRequestRefresh;

    bool IsSetup();
};