// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "chatPluginCommands.h"

#define LOCTEXT_NAMESPACE "FchatPluginModule"

void FchatPluginCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "chatPlugin", "Bring up chatPlugin window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
