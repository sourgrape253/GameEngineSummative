// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

//
// Bachelor of Software Engineering
// Media Design School
// Auckland
// New Zealand
//
//
// File Name	: chatPlugin.cpp
// Description	: chatPlugin implementation file.
// Authors		: Charmaine Lim, Matthew Seymour, Joseph Newman
//

#include "chatPlugin.h"
#include "chatPluginStyle.h"
#include "chatPluginCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

static const FName chatPluginTabName("chatPlugin");

#define LOCTEXT_NAMESPACE "FchatPluginModule"

//called prior to shutting down the module
void FchatPluginModule::PreUnloadCallback()
{
	if (m_bConnected && !m_bServer)
	{
		UDPSender_SendString(FString(TEXT("I have left the chat. Good Bye.")));
	}
	//if (m_bConnected && m_bServer)
	//{
	//	std::map<FString, TClientDetails>::iterator iter = m_pConnectedClients->begin();
	//	while (iter != m_pConnectedClients->end())
	//	{
	//		UDPSender_SendString(FString(TEXT("I have left the chat. Good Bye.")), iter->second.m_RemoteAddr, FName(*m_myName));
	//		++iter;
	//	}
	//}
}

//initialise everything
void FchatPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FchatPluginStyle::Initialize();
	FchatPluginStyle::ReloadTextures();

	FchatPluginCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FchatPluginCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FchatPluginModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FchatPluginModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FchatPluginModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(chatPluginTabName, FOnSpawnTab::CreateRaw(this, &FchatPluginModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FchatPluginTabTitle", "chatPlugin"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	SenderSocket = NULL;
	ListenSocket = NULL;
	m_bConnected = false;
	m_pConnectedClients = new std::map<FString, TClientDetails>;
}

//clear all pointers before shutdown
void FchatPluginModule::ShutdownModule()
{
	if (SenderSocket)
	{
		SenderSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
	}

	if (UDPReceiver)
	{
		UDPReceiver->Stop();
		delete UDPReceiver;
		UDPReceiver = nullptr;
	}

	if (ListenSocket)
	{
		ListenSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
	}

	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FchatPluginStyle::Shutdown();

	FchatPluginCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(chatPluginTabName);
}

//creates the chat window and gets the local ip address
TSharedRef<SDockTab> FchatPluginModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	bool canBind = false;
	TSharedRef<FInternetAddr> localIp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, canBind);

	if (localIp->IsValid())
	{
		MyReceivingIP = localIp->ToString(false);
	}

	TSharedRef<SDockTab> newTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Left)
			.Padding(15)
			[
				SAssignNew(ChatWidget, SChatWidget)
			]
		];

	ChatWidget->m_UDPinstance = this;
	
	ChatWidget->ChatInput->SetHintText(m_hint);

	return newTab;
}

//opens the chat window
void FchatPluginModule::PluginButtonClicked()
{
	bWidgetStarted = false;

	FGlobalTabmanager::Get()->InvokeTab(chatPluginTabName);
}

void FchatPluginModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FchatPluginCommands::Get().OpenPluginWindow);
}

void FchatPluginModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FchatPluginCommands::Get().OpenPluginWindow);
}

// creates the internet address
TSharedPtr<FInternetAddr> FchatPluginModule::CreateRemoteAddress(FString ip, int32 port, bool &bIsValid)
{
	TSharedPtr<FInternetAddr> _RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	_RemoteAddr->SetIp(*ip, bIsValid);
	_RemoteAddr->SetPort(port);

	return _RemoteAddr;
}

//creates the sending socket
void FchatPluginModule::CreateSenderSocket(const FString& YourChosenSocketName)
{
	SenderSocket = FUdpSocketBuilder(*YourChosenSocketName)
		.AsReusable()
		.WithBroadcast()
		;

	int32 SendSize = 2 * 1024 * 1024;
	SenderSocket->SetSendBufferSize(SendSize, SendSize);
	SenderSocket->SetReceiveBufferSize(SendSize, SendSize);
}

//creates the remote address and the sending socket
bool FchatPluginModule::StartUDPSender(const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort)
{
	bool bIsValid;

	RemoteAddr = CreateRemoteAddress(TheIP, ThePort, bIsValid);

	if (!bIsValid)
	{
		return false;
	}

	CreateSenderSocket(YourChosenSocketName);

	return true;
}

//sends a string to an address
bool FchatPluginModule::UDPSender_SendString(FString ToSend)
{
	return UDPSender_SendString(ToSend, RemoteAddr, *m_myName);
}

//sends a string, given an address and a name
bool FchatPluginModule::UDPSender_SendString(FString ToSend, TSharedPtr<FInternetAddr> _RemoteAddr, FName _name)
{
	if (!SenderSocket)
	{
		return false;
	}

	ESocketConnectionState state = SenderSocket->GetConnectionState();
	if (state != ESocketConnectionState::SCS_Connected)
	{
		return false;
	}

	FIPv4Address Addr;
	FIPv4Address::Parse(MyReceivingIP, Addr);
	FIPv4Endpoint Endpoint(Addr, MyReceivingPort);

	FAnyCustomData NewData;
	NewData.Name = _name;
	NewData.Message = ToSend;
	NewData.Port = MyReceivingPort;
	NewData.ipAddress = Endpoint;

	FArrayWriter Writer;
	Writer << NewData;

	int32 BytesSent = 0;
	SenderSocket->SendTo(Writer.GetData(), Writer.Num(), BytesSent, *_RemoteAddr);
	
	if (BytesSent <= 0)
	{
		return false;
	}
	
	return true;
}

//creates listening socket and receiver
bool FchatPluginModule::StartUDPReceiver(const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort)
{
	FIPv4Address Addr;
	FIPv4Address::Parse(TheIP, Addr);
	FIPv4Endpoint Endpoint(Addr, ThePort);

	int32 BufferSize = 2 * 1024 * 1024;

	ListenSocket = FUdpSocketBuilder(*YourChosenSocketName)
		.AsNonBlocking()
		//.AsReusable()
		.BoundToEndpoint(Endpoint)
		.WithReceiveBufferSize(BufferSize);

	if (!ListenSocket)
	{
		return false;
	}

	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	UDPReceiver = new FUdpSocketReceiver(ListenSocket, ThreadWaitTime, TEXT("UDP RECEIVER"));

	if (!UDPReceiver)
	{
		return false;
	}

	UDPReceiver->OnDataReceived().BindRaw(this, &FchatPluginModule::Recv);

	UDPReceiver->Start();

	return true;
}

// receive function handles the messages received
void FchatPluginModule::Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt)
{
	FAnyCustomData Data;
	*ArrayReaderPtr << Data;

	if (m_bServer)
	{
		std::pair<std::map<FString, TClientDetails>::iterator, bool> ret;

		bool bIsValid;

		TSharedPtr<FInternetAddr> _RemoteAddr = CreateRemoteAddress(Data.ipAddress.ToString(), Data.Port, bIsValid);

		TClientDetails _clientDetails;
		_clientDetails.m_endPoint = EndPt;
		_clientDetails.m_strName = Data.Name.ToString();
		_clientDetails.m_RemoteAddr = _RemoteAddr;

		ret = m_pConnectedClients->insert(std::pair<FString, TClientDetails>(EndPt.ToString(), _clientDetails));

		if (ret.second == false) 
		{
			SendToAll(Data, EndPt.ToString(), ret.second);
			
		}
		else
		{
			ChatMessage(FText::FromString(Data.Name.ToString()), FText::FromString("Hi Server. I just joined the chat!!! :)"));

			SendToAll(Data, EndPt.ToString(), ret.second);
		}
	}
	else
	{
		ChatMessage(FText::FromString(Data.Name.ToString()), FText::FromString(Data.Message));
	}
}

//send message to everyone
void FchatPluginModule::SendToAll(FAnyCustomData Data, FString address, bool handshake)
{
	std::map<FString, TClientDetails>::iterator iter = m_pConnectedClients->begin();
	iter = m_pConnectedClients->find(address);
	if (iter != m_pConnectedClients->end())
	{
		if (m_pConnectedClients->size() > 0)
		{
			std::map<FString, TClientDetails>::iterator iter2 = m_pConnectedClients->begin();
			while (iter2 != m_pConnectedClients->end())
			{
				if (address != iter2->first)
				{
					if (!handshake)
					{
						UDPSender_SendString(Data.Message, iter2->second.m_RemoteAddr, FName(*iter->second.m_strName));
					}
					else
					{
						UDPSender_SendString(Data.Name.ToString() + FString(TEXT(" just joined the chat.")), iter2->second.m_RemoteAddr, FName(*m_myName));
					}
				}
				else if (handshake)
				{
					UDPSender_SendString(FString(TEXT("Welcome to the chat ")) + Data.Name.ToString(), iter2->second.m_RemoteAddr, FName(*m_myName));
				}
				++iter2;
			}
		}
	}
}

// add the message in the chat window
void FchatPluginModule::ChatMessage(FText userName, FText message)
{
	FSChatMsg newmessage; 
	newmessage.Init(1, userName, message); 
	ChatWidget->AddMessage(newmessage);
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FchatPluginModule, chatPlugin)