// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

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

void FchatPluginModule::ShutdownModule()
{
	//FString message = m_myName + FString(TEXT(" just left the chat."));
	//UDPSender_SendString(message);

	if (SenderSocket)
	{
		SenderSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
	}

	UE_LOG(LogTemp, Warning, TEXT("ShutdownModule"));

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

TSharedRef<SDockTab> FchatPluginModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	bool canBind = false;
	TSharedRef<FInternetAddr> localIp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, canBind);

	if (localIp->IsValid())
	{
		GLog->Log(localIp->ToString(false)); // if you want to append the port (true) or not (false).
		UE_LOG(LogTemp, Warning, TEXT("%s"), *localIp->ToString(true)  );
		MyReceivingIP = localIp->ToString(false);
	}

	return SNew(SDockTab)
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
}

void FchatPluginModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->InvokeTab(chatPluginTabName);

	ChatWidget->m_UDPinstance = this;
}

void FchatPluginModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FchatPluginCommands::Get().OpenPluginWindow);
}

void FchatPluginModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FchatPluginCommands::Get().OpenPluginWindow);
}

bool FchatPluginModule::StartUDPSender(const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort)
{
	RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	bool bIsValid;
	RemoteAddr->SetIp(*TheIP, bIsValid);
	RemoteAddr->SetPort(ThePort);

	if (!bIsValid)
	{
		ScreenMsg(" UDP Sender>> IP address was not valid! ", TheIP);
		UE_LOG(LogTemp, Warning, TEXT("IP address was not valid!"));
		return false;
	}

	SenderSocket = FUdpSocketBuilder(*YourChosenSocketName)
		.AsReusable()
		.WithBroadcast()
		;

	int32 SendSize = 2 * 1024 * 1024;
	SenderSocket->SetSendBufferSize(SendSize, SendSize);
	SenderSocket->SetReceiveBufferSize(SendSize, SendSize);

	UE_LOG(LogTemp, Warning, TEXT("\n\n\n~~~~~~~~~~~~~StartUDPSender~~~~~~~~~~~~~~~~\n\n\n"));
	UE_LOG(LogTemp, Warning, TEXT("%i"), ThePort);


	return true;
}

bool FchatPluginModule::UDPSender_SendString(FString ToSend)
{
	if (!SenderSocket)
	{
		ScreenMsg("No sender socket");
		UE_LOG(LogTemp, Warning, TEXT("No sender socket"));
		return false;
	}

	FIPv4Address Addr;
	FIPv4Address::Parse(MyReceivingIP, Addr);

	//Create Socket
	FIPv4Endpoint Endpoint(Addr, MyReceivingPort);

	FAnyCustomData NewData;
	NewData.Name = *m_myName;
	NewData.Message = ToSend;
	NewData.Port = MyReceivingPort;
	NewData.ipAddress = Endpoint;

	FArrayWriter Writer;
	Writer << NewData; 


	UE_LOG(LogTemp, Warning, TEXT("%s"), *NewData.Name.ToString());
	int32 BytesSent = 0;
	SenderSocket->SendTo(Writer.GetData(), Writer.Num(), BytesSent, *RemoteAddr);

	if (BytesSent <= 0)
	{
		const FString Str = "Socket is valid but the receiver received 0 bytes, make sure it is listening properly!";
		UE_LOG(LogTemp, Error, TEXT("%s"), *Str);
		ScreenMsg(Str);
		return false;
	}

	ScreenMsg("UDP~ Send Succcess! Bytes Sent = ", BytesSent);

	UE_LOG(LogTemp, Warning, TEXT("Remote Address PORT"));
	UE_LOG(LogTemp, Warning, TEXT("%i"), RemoteAddr->GetPort());

	UE_LOG(LogTemp, Warning, TEXT("Remote Address Platform PORT"));
	UE_LOG(LogTemp, Warning, TEXT("%i"), RemoteAddr->GetPlatformPort());

	return true;
}

//used by the server
bool FchatPluginModule::UDPSender_SendString(FString ToSend, TSharedPtr<FInternetAddr> _RemoteAddr, FName _name)
{
	if (!SenderSocket)
	{
		UE_LOG(LogTemp, Warning, TEXT("No sender socket"));
		return false;
	}

	int32 BytesSent = 0;

	FAnyCustomData NewData;
	NewData.Name = _name;
	NewData.Message = ToSend;
	//NewData.Port = MyReceivingPort;

	FArrayWriter Writer;
	Writer << NewData;

	SenderSocket->SendTo(Writer.GetData(), Writer.Num(), BytesSent, *_RemoteAddr);

	if (BytesSent <= 0)
	{
		const FString Str = "Socket is valid but the receiver received 0 bytes, make sure it is listening properly!";
		UE_LOG(LogTemp, Error, TEXT("%s"), *Str);
		ScreenMsg(Str);
		return false;
	}

	return true;
}

bool FchatPluginModule::StartUDPReceiver(const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort)
{
	//ScreenMsg("RECEIVER INIT");

	FIPv4Address Addr;
	FIPv4Address::Parse(TheIP, Addr);

	//Create Socket
	FIPv4Endpoint Endpoint(Addr, ThePort);

	//BUFFER SIZE
	int32 BufferSize = 2 * 1024 * 1024;

	ListenSocket = FUdpSocketBuilder(*YourChosenSocketName)
		.AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.WithReceiveBufferSize(BufferSize);

	if (!ListenSocket)
	{
		UE_LOG(LogTemp, Error, TEXT("Listen Socket Error"));
	}

	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	UDPReceiver = new FUdpSocketReceiver(ListenSocket, ThreadWaitTime, TEXT("UDP RECEIVER"));

	if (!UDPReceiver)
	{
		UE_LOG(LogTemp, Error, TEXT("UDP Receiver Error"));
	}

	UE_LOG(LogTemp, Warning, TEXT("new socket"));

	UDPReceiver->OnDataReceived().BindRaw(this, &FchatPluginModule::Recv);

	UDPReceiver->Start();

	UE_LOG(LogTemp, Warning, TEXT("Created udp Receiver"));

	return true;
}

void FchatPluginModule::Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt)
{
	UE_LOG(LogTemp, Warning, TEXT("Received Something"));
	ScreenMsg("Received bytes", ArrayReaderPtr->Num());

	FAnyCustomData Data;
	*ArrayReaderPtr << Data;

	//print it out
	UE_LOG(LogTemp, Error, TEXT("%s"), *Data.Name.ToString());
	ScreenMsg("Received message:", *Data.Name.ToString());

	//UE_LOG(LogTemp, Warning, TEXT("Endpoint TO String"));
	//UE_LOG(LogTemp, Warning, TEXT("%s"), *EndPt.ToString());
	//UE_LOG(LogTemp, Error, TEXT("Endpoint TO Text"));
	//UE_LOG(LogTemp, Error, TEXT("%s"), *EndPt.ToText().ToString());
	//127.0.0.1:57934
	
	if (m_bServer)
	{
		std::pair<std::map<FString, TClientDetails>::iterator, bool> ret;

		TSharedPtr<FInternetAddr> _RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

		bool isValid;
		_RemoteAddr->SetIp(*Data.ipAddress.ToString(), isValid);
		_RemoteAddr->SetPort(Data.Port);

		TClientDetails _clientDetails;
		_clientDetails.m_endPoint = EndPt;
		_clientDetails.m_strName = Data.Name.ToString();
		_clientDetails.m_RemoteAddr = _RemoteAddr;

		ret = m_pConnectedClients->insert(std::pair<FString, TClientDetails>(EndPt.ToString(), _clientDetails));

		UE_LOG(LogTemp, Error, TEXT("Endpoint TO Text"));
		UE_LOG(LogTemp, Error, TEXT("%s"), *EndPt.ToText().ToString());

		if (ret.second == false) 
		{
			//send data to everyone except the sender
			
			UE_LOG(LogTemp, Warning, TEXT("Insert into map failed because client exists in map"));

			std::map<FString, TClientDetails>::iterator iter = m_pConnectedClients->begin();
			iter = m_pConnectedClients->find(EndPt.ToString());

			if (iter != m_pConnectedClients->end())
			{

				if (m_pConnectedClients->size() > 0)
				{
					std::map<FString, TClientDetails>::iterator iter2 = m_pConnectedClients->begin();
					while (iter2 != m_pConnectedClients->end())
					{
						if (EndPt.ToString() != iter2->first)
						{
							UDPSender_SendString(Data.Message, iter2->second.m_RemoteAddr, FName(*iter->second.m_strName));
						}
						++iter2;
					}
				}
			}
		}
		else
		{
			//RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
			//
			//RemoteAddr = EndPt.ToInternetAddr();
			//RemoteAddr->SetPort(Data.Port);

			CreateSenderSocket(FString(TEXT("theirSocket")));

			UE_LOG(LogTemp, Warning, TEXT("Insert into map success"));

			FSChatMsg newmessage; // make a new struct to send for replication
			newmessage.Init(1, FText::FromString(Data.Name.ToString()), FText::FromString("Hi Server. I just joined the chat!!! :)")); // initialize the message struct for replication
			ChatWidget->AddMessage(newmessage);


			std::map<FString, TClientDetails>::iterator iter = m_pConnectedClients->begin();
			iter = m_pConnectedClients->find(EndPt.ToString());

			if (iter != m_pConnectedClients->end())
			{
				if (m_pConnectedClients->size() > 0)
				{
					if (m_pConnectedClients->size() > 0)
					{
						std::map<FString, TClientDetails>::iterator iter2 = m_pConnectedClients->begin();
						while (iter2 != m_pConnectedClients->end())
						{
							FString message;
							if (EndPt.ToString() != iter2->first)
							{
								message = Data.Name.ToString() + FString(TEXT(" just joined the chat."));
								UDPSender_SendString(message, iter2->second.m_RemoteAddr, FName(*m_myName));
							}
							else
							{
								message = FString(TEXT("Welcome to the chat ")) + Data.Name.ToString();
								UDPSender_SendString(message, iter2->second.m_RemoteAddr, FName(*m_myName));
							}
							++iter2;
						}
					}
				}
			}



		}

		//return true;
	}
	else
	{
		FSChatMsg newmessage; // make a new struct to send for replication
		newmessage.Init(1, FText::FromString(Data.Name.ToString()), FText::FromString(Data.Message)); // initialize the message struct for replication
		ChatWidget->AddMessage(newmessage);
	}
}

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



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FchatPluginModule, chatPlugin)