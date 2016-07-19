// Copyright 2016 dotBunny, Inc. All Rights Reserved.
#pragma once

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSettings.generated.h"

UCLASS(config = EditorUserSettings, defaultconfig)
class UCLionSettings : public UObject
{
    GENERATED_UCLASS_BODY()

    UPROPERTY(Config, EditAnywhere, Category="CLion", Meta=(DisplayName="Path To CLion Executable"))
    FFilePath CLionPath;

    UPROPERTY(Config, EditAnywhere, Category="Compiler", Meta=(DisplayName="Path To CLang++ Executable"))
    FFilePath CLangXXPath;

    UPROPERTY(Config, EditAnywhere, Category="Compiler", Meta=(DisplayName="Path To CLang Executable"))
    FFilePath CLangPath;

    UPROPERTY(Config, EditAnywhere, Category="Project", Meta=(DisplayName="Project Root Folder"))
    FDirectoryPath ProjectPath;


    UPROPERTY(Config, EditAnywhere, Category="Project", Meta=(DisplayName="Unreal Engine Root Folder"))
    FDirectoryPath EnginePath;


    UPROPERTY(Config, EditAnywhere, Category="CMake", Meta=(DisplayName="Template",MultiLine="true"))
    FText CMakeTemplate;


protected:
#if WITH_EDITOR
    virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent ) override;
	virtual void PreEditChange(UProperty* PropertyAboutToChange) override;
#endif

private:
    FString PreviousProjectPath;
    FString PreviousEnginePath;
    FString PreviousCLionPath;
    FString PreviousCLangPath;
    FString PreviousCLangXXPath;

    FText PreviousCMakeTemplate;

    bool CheckSetup();
    bool bSetupComplete = false;

public:
    bool bRequestRefresh;

    bool IsSetup();
    bool OutputCMakeList();
};