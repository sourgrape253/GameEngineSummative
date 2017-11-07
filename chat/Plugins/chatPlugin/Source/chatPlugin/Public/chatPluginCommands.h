// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "chatPluginStyle.h"

class FchatPluginCommands : public TCommands<FchatPluginCommands>
{
public:

	FchatPluginCommands()
		: TCommands<FchatPluginCommands>(TEXT("chatPlugin"), NSLOCTEXT("Contexts", "chatPlugin", "chatPlugin Plugin"), NAME_None, FchatPluginStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};