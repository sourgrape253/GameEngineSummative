//
// Bachelor of Software Engineering
// Media Design School
// Auckland
// New Zealand
//
// (c) 2016 Media Design School
//
// File Name	: SChatWidget.h
// Description	: SChatWidget declaration file.
// Authors		: Charmaine Lim, Matthew Seymour, Joseph Newman
//

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "SlateBasics.h"

unsigned const DEFAULT_SERVER_PORT = 20000; 

struct FSChatMsg 
{
	UPROPERTY() 
		int32 Type;

	UPROPERTY()
		FText Username;

	UPROPERTY()
		FText Text;

	FText Timestamp; 
	double Created;

	void Init(int32 NewType, FText NewUsername, FText NewText) 
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

	void Construct(const FArguments& InArgs);

	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FSChatMsg> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TArray<TSharedPtr<FSChatMsg>> Items; 
	TSharedPtr< SListView< TSharedPtr<FSChatMsg> > > ListViewWidget; 
	TSharedPtr< SVerticalBox > ChatBox;
	TSharedPtr< SEditableText > ChatInput;

	void OnChatTextChanged(const FText& InText);
	void OnChatTextCommitted(const FText& InText, ETextCommit::Type CommitMethod);

	void AddMessage(const FSChatMsg& newmessage); 

	FchatPluginModule* m_UDPinstance;
};
