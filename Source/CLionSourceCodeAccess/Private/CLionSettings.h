// Copyright 2016 dotBunny, Inc. All Rights Reserved.
#pragma once

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSettings.generated.h"

UCLASS(config = Editor, defaultconfig)
class UCLionSettings : public UObject
{
    GENERATED_UCLASS_BODY()

    UPROPERTY(Config, EditAnywhere, Category="CLionSetup", Meta=(DisplayName="Path To CLion Executable"))
    FFilePath CLionPath;

    UPROPERTY(Config, EditAnywhere, Category="ProjectSetup", Meta=(DisplayName="Project Folder"))
    FDirectoryPath ProjectPath;

    UPROPERTY(Config, EditAnywhere, Category="ProjectSetup", Meta=(DisplayName="Unreal Engine Source Folder"))
    FDirectoryPath EnginePath;

    UPROPERTY(Config, EditAnywhere, Category="ProjectSetup", Meta=(DisplayName="CMakeList Template"))
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
    FText PreviousCMakeTemplate;

    bool CheckSetup();
    bool bSetupComplete = false;

public:
    bool bRequestRefresh;

    bool IsSetup();
    bool OutputCMakeList();
};