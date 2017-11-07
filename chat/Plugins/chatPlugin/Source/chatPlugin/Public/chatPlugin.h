// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"
#include "Networking.h"
#include "Engine.h"
#include "SlateBasics.h"
#include "SChatWidget.h"

class FToolBarBuilder;
class FMenuBuilder;

struct FAnyCustomData
{
	FString Name = "GREAT!";

	FAnyCustomData()
	{}
};

FORCEINLINE FArchive& operator<<(FArchive &Ar, FAnyCustomData& TheStruct)
{
	Ar << TheStruct.Name;

	return Ar;
}

class FchatPluginModule : public IModuleInterface
{
public:

	//*****************************************Sending*****************************************
	bool UDPSender_SendString(FString ToSend);

	TSharedPtr<FInternetAddr> RemoteAddr;
	FSocket* SenderSocket;

	bool StartUDPSender(const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort);


	//*****************************************Receiving*****************************************
	FSocket* ListenSocket;

	FUdpSocketReceiver* UDPReceiver = nullptr;

	void Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);

	bool StartUDPReceiver(const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort);

	//*****************************************Others*****************************************

	TSharedPtr<SChatWidget> ChatWidget;

	FString TheirReceiverIP;
	FString MyReceivingIP;

	int32 MyReceivingPort;
	int32 TheirReceiverPort;

	bool m_bConnected;

	//*****************************************Debug*****************************************
	FORCEINLINE void ScreenMsg(const FString& Msg)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, *Msg);
	}
	FORCEINLINE void ScreenMsg(const FString& Msg, const float Value)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s %f"), *Msg, Value));
	}
	FORCEINLINE void ScreenMsg(const FString& Msg, const FString& Msg2)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s %s"), *Msg, *Msg2));
	}


	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
private:

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};