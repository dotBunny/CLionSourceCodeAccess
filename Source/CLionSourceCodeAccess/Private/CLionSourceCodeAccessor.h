// Copyright 2017 dotBunny Inc. All Rights Reserved.

#pragma once

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "ISourceCodeAccessor.h"
#include "XmlParser.h"
#include "CLionSettings.h"

class FCLionSourceCodeAccessor : public ISourceCodeAccessor
{
public:
	/** ISourceCodeAccessor implementation */
	virtual void RefreshAvailability() override
	{
	}

	virtual bool CanAccessSourceCode() const override;

	virtual FName GetFName() const override;

	virtual FText GetNameText() const override;

	virtual FText GetDescriptionText() const override;

	virtual bool OpenSolution() override;

	virtual bool OpenFileAtLine(const FString& FullPath, int32 LineNumber, int32 ColumnNumber = 0) override;

	virtual bool OpenSourceFiles(const TArray<FString>& AbsoluteSourcePaths) override;

	virtual bool
	AddSourceFiles(const TArray<FString>& AbsoluteSourcePaths, const TArray<FString>& AvailableModules) override;

	virtual bool SaveAllOpenDocuments() const override;

	/**
	 * Frame Tick (Not Used)
	 * @param DeltaTime of frame
	 */
	virtual void Tick(const float DeltaTime) override
	{
	}

	/**
 	* Create the CMakeLists.txt file and all the sub *.cmake addins.
 	*/
	void GenerateProjectFile();

	/**
 	* Deinitialize the accessor.
 	*/
	void Shutdown();

	/**
	 * Initialize the accessor.
	 */
	void Startup();

private:

	/**
	 * A local storage of the working Project name as we parse files
	 */
	FString WorkingProjectName;

	/**
     * A local reference to the Settings object.
     */
	UCLionSettings* Settings;

	/**
	 * A local storage of the working Mono path found while parsing files
	 */
	FString WorkingMonoPath;



	/**
	 * Instruct UnrealBuildTool to generate a CodeLite project, then convert it to CMakeList
	 * @return Was the project generation successful?
	 */
	bool GenerateFromCodeLiteProject();

	/**
	 * Recursively search XmlNode for children namd Tag, and grab their Attribute.
	 * @param The root XmlNode to search from.
	 * @param The tag of the elements to uses.
	 * @param The attribute that we want to collect.
	 * @return A CMakeList compatible string set of the attributes.
	 */
	FString GetAttributeByTagWithRestrictions(FXmlNode* CurrentNode, const FString& Tag, const FString& Attribute);

	FString GetBuildCommands(FXmlNode* CurrentNode, const FString& SubprojectName);

	FString HandleConfiguration(FXmlNode* CurrentNode, const FString& SubprojectName);
};