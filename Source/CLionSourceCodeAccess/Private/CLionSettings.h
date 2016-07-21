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
    FString PreviousCLangPath;
    FString PreviousCLangXXPath;
    FString PreviousMonoPath;


    bool CheckSetup();

    bool bSetupComplete = false;

public:
    bool bRequestRefresh;

    bool IsSetup();
};