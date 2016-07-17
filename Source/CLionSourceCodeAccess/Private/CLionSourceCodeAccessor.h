// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
// Copyright 2016 dotBunny, Inc. All Rights Reserved.

#pragma once

#include "ISourceCodeAccessor.h"

class FCLionSourceCodeAccessor : public ISourceCodeAccessor
{
public:
	/** ISourceCodeAccessor implementation */
	virtual void RefreshAvailability() override { }
	virtual bool CanAccessSourceCode() const override;
	virtual FName GetFName() const override;
	virtual FText GetNameText() const override;
	virtual FText GetDescriptionText() const override;
	virtual bool OpenSolution() override;
	virtual bool OpenFileAtLine(const FString& FullPath, int32 LineNumber, int32 ColumnNumber = 0) override;
	virtual bool OpenSourceFiles(const TArray<FString>& AbsoluteSourcePaths) override;
	virtual bool AddSourceFiles(const TArray<FString>& AbsoluteSourcePaths, const TArray<FString>& AvailableModules) override;
	virtual bool SaveAllOpenDocuments() const override;
	virtual void Tick(const float DeltaTime) override;


	/**
 	* Create the CMakeLists.txt File, called from File -> Refresh CMakeLists
 	*/
	void GenerateProjectFile();

	/**
	 * Get the path to the CLion executable
	 */
	FString GetCLionExecutable() const;

	/**
	 * Check if CLion is already running
	 */
	bool IsIDERunning();

	/**
	 * Open the CLion IDE, called from File ->Open CLion
	 */
	void OpenCLion();

	/**
 	* Deinitialize the accessor.
 	*/
	void Shutdown();

	/**
	 * Initialize the accessor.
	 */
	void Startup();

private:

	mutable FString CachedSolutionPath;
	mutable FString CachedCLionPath;

	bool CanRunCLion;

	FString GetSolutionPath() const;
};