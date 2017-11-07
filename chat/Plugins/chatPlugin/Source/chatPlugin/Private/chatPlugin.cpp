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
}

void FchatPluginModule::ShutdownModule()
{
	if (SenderSocket)
	{
		SenderSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
	}

	UE_LOG(LogTemp, Warning, TEXT("ShutdownModule"));

	UDPReceiver->Stop();

	delete UDPReceiver;
	UDPReceiver = nullptr;

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

	int32 BytesSent = 0;

	FAnyCustomData NewData;
	NewData.Name = ToSend;

	FArrayWriter Writer;
	Writer << NewData; 

	UE_LOG(LogTemp, Warning, TEXT("%s"), *NewData.Name);

	SenderSocket->SendTo(Writer.GetData(), Writer.Num(), BytesSent, *RemoteAddr);

	if (BytesSent <= 0)
	{
		const FString Str = "Socket is valid but the receiver received 0 bytes, make sure it is listening properly!";
		UE_LOG(LogTemp, Error, TEXT("%s"), *Str);
		ScreenMsg(Str);
		return false;
	}

	ScreenMsg("UDP~ Send Succcess! Bytes Sent = ", BytesSent);

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


	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	UDPReceiver = new FUdpSocketReceiver(ListenSocket, ThreadWaitTime, TEXT("UDP RECEIVER"));

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
	UE_LOG(LogTemp, Error, TEXT("%s"), *Data.Name);
	ScreenMsg("Received message:", *Data.Name);

	FSChatMsg newmessage; // make a new struct to send for replication
	newmessage.Init(1, FText::FromString("Friend"), FText::FromString(Data.Name)); // initialize the message struct for replication
	ChatWidget->AddMessage(newmessage);
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FchatPluginModule, chatPlugin)