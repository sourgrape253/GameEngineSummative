// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "SlateBasics.h"

unsigned const DEFAULT_SERVER_PORT = 50012; //Changed the default server port so that it does not clash with client port during broadcast scanning for servers
unsigned const DEFAULT_CLIENT_PORT = 60013;

struct FSChatMsg // Struct to hold the message data to be passed between classes
{
	UPROPERTY() // UProperty means this variable will be replicated
		int32 Type;

	UPROPERTY()
		FText Username;

	UPROPERTY()
		FText Text;

	FText Timestamp; // Dont replicate time because we can set it locally once we receive the struct

	double Created;

	void Init(int32 NewType, FText NewUsername, FText NewText) // Assign only the vars we wish to replicate
	{
		Type = NewType;
		Username = NewUsername;
		Text = NewText;
	}

	void SetTime(FText NewTimestamp, double NewCreated)
	{
		Timestamp = NewTimestamp;
		Created = NewCreated;
	}

	void Destroy()
	{
		Type = NULL;
		Username.GetEmpty();
		Text.GetEmpty();
		Timestamp.GetEmpty();
		Created = NULL;
	}
};

class FchatPluginModule;

/**
 * 
 */
class CHATPLUGIN_API SChatWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SChatWidget)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FSChatMsg> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TArray<TSharedPtr<FSChatMsg>> Items; 
	TSharedPtr< SListView< TSharedPtr<FSChatMsg> > > ListViewWidget; 
	TSharedPtr< SVerticalBox > ChatBox;
	TSharedPtr< SEditableText > ChatInput;

	void OnChatTextChanged(const FText& InText);
	void OnChatTextCommitted(const FText& InText, ETextCommit::Type CommitMethod);

	void AddMessage(const FSChatMsg& newmessage); // the final stage, this function takes the input and does the final placement in the chatbox

	FchatPluginModule* m_UDPinstance;

	int count = 0;
};
