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
     * Does the project files require a refresh based on changes made to the settings?
     */
    bool bRequireRefresh = false;

    /**
     * Check our settings, cache results and return
     * @return Can the plugin be used?
     */
    bool CheckSettings();

	/**
 	* Get the cached CMakeList path.
 	* @return CMakeList Path
 	*/
	FString GetCMakeListPath();

    /**
    * Are the settings for the plugin complete and usable?
    */
    bool IsSetup();

    /**
     * [optional] Path to a C compiler to be used in the CMakeList file.
     */
    UPROPERTY(Config, EditAnywhere, Category="CMakeList", Meta=(DisplayName="C Compiler (Optional)"))
    FFilePath CCompiler;

    /**
     * Path to CLion executable, used when needing to launch CLion.
     */
    UPROPERTY(Config, EditAnywhere, Category="CLion", Meta=(DisplayName="CLion Executable"))
    FFilePath CLion;

    /**
     * [optional] Path to a C++ compiler to be used in the CMakeList file."
     */
    UPROPERTY(Config, EditAnywhere, Category="CMakeList", Meta=(DisplayName="C++ Compiler (Optional)"))
    FFilePath CXXCompiler;


	/**
	 * Include Config In Makefile
	 */
	UPROPERTY(Config, EditAnywhere, Category="CMakeList", Meta=(DisplayName="Include Configs"))
	bool bIncludeConfigs = false;

	/**
	 * Include Plugins In Makefile
	 */
	UPROPERTY(Config, EditAnywhere, Category="CMakeList", Meta=(DisplayName="Include Plugins"))
	bool bIncludePlugins = false;


	/**
	 * Include Shaders In Makefile
	 */
	UPROPERTY(Config, EditAnywhere, Category="CMakeList", Meta=(DisplayName="Include Shaders"))
	bool bIncludeShaders = false;

    /**
     * Path to the Mono executable.
     */
    UPROPERTY(Config, EditAnywhere, Category="Unreal", Meta=(DisplayName="Mono Executable"))
    FFilePath Mono;

protected:
#if WITH_EDITOR
    virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent ) override;
	virtual void PreEditChange(UProperty* PropertyAboutToChange) override;
#endif

private:

    /**
     * Cached flag to tell if all settings are present needed to use the plugin.
     */
    bool bSetupComplete = false;

	/**
 	* A cached reference of the path to the CMakeList.txt file
 	*/
	FString CachedCMakeListPath;

	/**
     * Cached version of C Compiler location.
     */
	FString PreviousCCompiler;

	/**
     * Cached version of CLion location.
     */
	FString PreviousCLion;

	/**
     * Cached version of C++ Compiler location.
     */
	FString PreviousCXXCompiler;

    /**
     * Cached version of Mono location.
     */
    FString PreviousMono;
};