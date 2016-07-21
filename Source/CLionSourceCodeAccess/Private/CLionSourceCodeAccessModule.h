// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
// Copyright 2016 dotBunny, Inc. All Rights Reserved.

#pragma once

#include "CLionSourceCodeAccessPrivatePCH.h"
#include "CLionSourceCodeAccessor.h"

class FCLionSourceCodeAccessModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
    virtual bool SupportsDynamicReloading() override;

    void AddMenuOptions(FMenuBuilder& MenuBuilder);


private:
    TSharedPtr<FExtender> MainMenuExtender;
	FCLionSourceCodeAccessor CLionSourceCodeAccessor;

    void RegisterSettings();
    void RegisterMenu();
	void UnregisterSettings();

    void HandleGenerateProjectFiles();
	void HandleOpenCLion();
};