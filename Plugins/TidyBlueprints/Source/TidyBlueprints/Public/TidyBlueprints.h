// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FTidyBlueprintsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Adds a menu item to the function call nodes
	 */
	void RegisterFunctionNodeMenuItem();

	/**
	 * Adds a menu item to the get and set variable nodes
	 */
	void RegisterVariableNodeMenuItem();
};
