// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

//
// Bachelor of Software Engineering
// Media Design School
// Auckland
// New Zealand
//
//
// File Name	: chatPlugin.h
// Description	: chatPlugin declaration file.
// Authors		: Charmaine Lim, Matthew Seymour, Joseph Newman
//

#pragma once

#include <map>

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
	FIPv4Endpoint ipAddress;
	int32 Port;
	FName Name = "myname!";
	FString Message = "GREAT to see you!";

	FAnyCustomData()
	{}
};

FORCEINLINE FArchive& operator<<(FArchive &Ar, FAnyCustomData& TheStruct)
{
	Ar << TheStruct.ipAddress;
	Ar << TheStruct.Port;
	Ar << TheStruct.Name;
	Ar << TheStruct.Message;
	return Ar;
}

struct TClientDetails
{
	FIPv4Endpoint m_endPoint;	
	FString m_strName;
	int32 m_iPort;
	bool m_bIsActive;
	TSharedPtr<FInternetAddr> m_RemoteAddr;
};

class FchatPluginModule : public IModuleInterface
{
public:

	//*****************************************Sending*****************************************
	bool UDPSender_SendString(FString ToSend);
	bool UDPSender_SendString(FString ToSend, TSharedPtr<FInternetAddr> _RemoteAddr, FName _name);

	TSharedPtr<FInternetAddr> RemoteAddr;
	FSocket* SenderSocket;

	bool StartUDPSender(const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort);

	void CreateSenderSocket(const FString& YourChosenSocketName);

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
	bool m_bServer;

	std::map<FString, TClientDetails>* m_pConnectedClients;

	FString m_myName;

	void ChatMessage(FText userName, FText message);
	TSharedPtr<FInternetAddr> CreateRemoteAddress(FString ip, int32 port, bool &bIsValid); 
	void SendToAll(FAnyCustomData Data, FString address, bool handshake);

	bool bIsServerClientChosen = false;
	bool bNameAdded = false;
	bool bPortAdded = false;
	bool bServerIPAdded = false;
	bool bWidgetStarted = false;

	FText m_hint = FText::FromString("Do you want to run a server or a client (C/S)?");

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual void PreUnloadCallback() override;
	//virtual bool SupportsAutomaticShutdown() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
private:

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};