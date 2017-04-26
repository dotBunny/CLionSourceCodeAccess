// Copyright 2017 dotBunny Inc. All Rights Reserved.

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
	UPROPERTY(Config, EditAnywhere, Category = "CMake Compiler", Meta = (DisplayName = "C Compiler (Optional)"))
	FFilePath CCompiler;

	/**
	 * Path to CLion executable, used when needing to launch CLion.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "CLion", Meta = (DisplayName = "CLion Executable"))
	FFilePath CLion;

	/**
	 * [optional] Path to a C++ compiler to be used in the CMakeList file."
	 */
	UPROPERTY(Config, EditAnywhere, Category = "CMake Compiler", Meta = (DisplayName = "C++ Compiler (Optional)"))
	FFilePath CXXCompiler;

	/**
	* Target Configuration Debug
	*/
	UPROPERTY(Config, EditAnywhere, Category = "CMake Target Configurations", Meta = (DisplayName = "Debug"))
	bool bConfigureDebug = false;

	/**
	 * Target Configuration DebugGame
	 */
	UPROPERTY(Config, EditAnywhere, Category = "CMake Target Configurations", Meta = (DisplayName = "DebugGame"))
	bool bConfigureDebugGame = false;

	/**
	* Target Configuration Development
	*/
	UPROPERTY(Config, EditAnywhere, Category = "CMake Target Configurations", Meta = (DisplayName = "Development"))
	bool bConfigureDevelopment = true;

	/**
	 * Target Configuration Shipping
	 */
	UPROPERTY(Config, EditAnywhere, Category = "CMake Target Configurations", Meta = (DisplayName = "Shipping"))
	bool bConfigureShipping = false;

	/**
	 * Target Configuration Test
	 */
	UPROPERTY(Config, EditAnywhere, Category = "CMake Target Configurations", Meta = (DisplayName = "Test"))
	bool bConfigureTest = false;

	/**
	 * Include Config In Makefile
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Additional Folders", Meta = (DisplayName = "Include Configs"))
	bool bIncludeConfigs = false;

	/**
	 * Include Plugins In Makefile
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Additional Folders", Meta = (DisplayName = "Include Plugins"))
	bool bIncludePlugins = false;

	/**
	 * Include Shaders In Makefile
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Additional Folders", Meta = (DisplayName = "Include Shaders"))
	bool bIncludeShaders = false;

	/**
	 * Target Project Game Editor
	 */
	UPROPERTY(Config, EditAnywhere, Category = "CMake Target Projects", Meta = (DisplayName = "Project Game"))
	bool bProjectSpecificGame = false;
	/**
	 * Target Project Specific Editor
	 */
	UPROPERTY(Config, EditAnywhere, Category = "CMake Target Projects", Meta = (DisplayName = "Project Editor"))
	bool bProjectSpecificEditor = true;

	/**
	 * Target Project UE4 Editor
	 */
	UPROPERTY(Config, EditAnywhere, Category = "CMake Target Projects", Meta = (DisplayName = "UE4 Editor"))
	bool bProjectUE4Editor = false;
	/**
	 * Target Project UE4 Game
	 */
	UPROPERTY(Config, EditAnywhere, Category = "CMake Target Projects", Meta = (DisplayName = "UE4 Game"))
	bool bProjectUE4Game = false;

	// TODO: We can't !PLATFORM_WINDOWS this as UBT/UHT barfs
	/**
	 * Path to the Mono executable (Required on non-Windows platforms).
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Unreal Engine", Meta = (DisplayName = "Mono Executable (Ignore On Windows)"))
	FFilePath Mono;

protected:
#if WITH_EDITOR

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

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