// Fill out your copyright notice in the Description page of Project Settings.

#include "SChatWidget.h"
#include "SlateOptMacros.h"
#include "chatPlugin.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SChatWidget::Construct(const FArguments& InArgs)
{
	ChildSlot 
	.VAlign(VAlign_Bottom)
	.HAlign(HAlign_Left)
	.Padding(15)  
	[
		SNew(SVerticalBox) 
		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(408.f)
		.VAlign(VAlign_Bottom)
		[
			SAssignNew(ListViewWidget, SListView< TSharedPtr< FSChatMsg > >) 
			.ListItemsSource(&Items) 
			.OnGenerateRow(this, &SChatWidget::OnGenerateRowForList) 
			.ScrollbarVisibility(EVisibility::Visible)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.FillHeight(30.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.MaxWidth(600.f)
			[
				SAssignNew(ChatInput, SEditableText) 
				.OnTextCommitted(this, &SChatWidget::OnChatTextCommitted) 
				.OnTextChanged(this, &SChatWidget::OnChatTextChanged) 
				.ClearKeyboardFocusOnCommit(true)
				.Text(FText::FromString(""))
				.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.9f)) 
				.HintText(FText::FromString("Please Enter your IP Address:")) 
			]
		]
	];
}

TSharedRef<ITableRow> SChatWidget::OnGenerateRowForList(TSharedPtr< FSChatMsg > Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!Items.IsValidIndex(0) || !Item.IsValid() || !Item.Get()) // Error catcher
		return
		SNew(STableRow< TSharedPtr< FSChatMsg > >, OwnerTable)
		[
			SNew(SBox)
		];

	if (Item.Get()->Type == 1) // Type 1 is for player chat messages
		return
		SNew(STableRow< TSharedPtr< FSChatMsg > >, OwnerTable)
		[
			SNew(SWrapBox)
			.PreferredWidth(600.f)
			+ SWrapBox::Slot()
			[
				SNew(STextBlock) // places the timestamp
				.Text(Item.Get()->Timestamp)
				.ColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.25f, 1.f))
				.ShadowColorAndOpacity(FLinearColor::Black)
				.ShadowOffset(FIntPoint(1, 1))
			]
			+ SWrapBox::Slot()
			[
				SNew(STextBlock) // places the username
				.Text(Item.Get()->Username)
				.ColorAndOpacity(FLinearColor::White)
				.ShadowColorAndOpacity(FLinearColor::Black)
				.ShadowOffset(FIntPoint(1, 1))
				]
			+ SWrapBox::Slot()
			[
				SNew(STextBlock) // adds the : between the username and chat text
				.Text(FText::FromString(" :  "))
				.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.f))
				.ShadowColorAndOpacity(FLinearColor::Black)
				.ShadowOffset(FIntPoint(1, 1))
			]
			+ SWrapBox::Slot()
			[
				SNew(STextBlock) // places the user text
				.Text(Item.Get()->Text)
				.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.f))
				.ShadowColorAndOpacity(FLinearColor::Black)
				.ShadowOffset(FIntPoint(1, 1))
			]
		];
	else // 2 is for server messages, add more types for whispers friendslists etc
		return
		SNew(STableRow< TSharedPtr< FSChatMsg > >, OwnerTable)
		[
			SNew(SWrapBox)
			.PreferredWidth(600.f)
			+ SWrapBox::Slot()
			[
				SNew(STextBlock)
				.Text(Item.Get()->Timestamp)
				.ColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.25f, 1.f))
				.ShadowColorAndOpacity(FLinearColor::Black)
				.ShadowOffset(FIntPoint(1, 1))
			]
			+ SWrapBox::Slot()
			[
				SNew(STextBlock)
				.Text(Item.Get()->Text)
				.ColorAndOpacity(FLinearColor(0.75f, 0.75f, 0.75f, 1.f))
				.ShadowColorAndOpacity(FLinearColor::Black)
				.ShadowOffset(FIntPoint(1, 1))
			]
		];
}

void SChatWidget::OnChatTextChanged(const FText& InText) 
{
	FString SText = InText.ToString();
	if (SText.Len() > 120) 
	{
		SText = SText.Left(120);
		if (ChatInput.IsValid())
			ChatInput->SetText(FText::FromString(SText));
	}
}

void SChatWidget::OnChatTextCommitted(const FText& InText, ETextCommit::Type CommitMethod) 
{
	if (CommitMethod != ETextCommit::OnEnter) 
		return;

	if (ChatInput.IsValid())
	{
		FText NFText = FText::TrimPrecedingAndTrailing(InText); 
		if (!NFText.IsEmpty())
		{
			FSChatMsg newmessage; 
			newmessage.Init(1, FText::FromString("Me"), NFText); 
			AddMessage(newmessage);

			count++;

			if (count == 1)
			{
				//uncomment later
				//m_UDPinstance->MyReceivingIP = NFText.ToString();
				m_UDPinstance->MyReceivingPort = FCString::Atoi(*NFText.ToString());

				FString MyChosenSocketName = FString(TEXT("mySocket"));
				//int32 MyReceivingPort = 60012;
				//m_UDPinstance->StartUDPReceiver(MyChosenSocketName, m_UDPinstance->MyReceivingIP, MyReceivingPort);

				FString MyReceivingIP = FString(TEXT("127.0.0.1"));
				m_UDPinstance->StartUDPReceiver(MyChosenSocketName, MyReceivingIP, m_UDPinstance->MyReceivingPort);

				ChatInput->SetHintText(FText::FromString("Please enter your friend's IP address: "));
			}
			else if (count == 2)
			{
				//m_UDPinstance->TheirReceiverIP = NFText.ToString();
				m_UDPinstance->TheirReceiverPort = FCString::Atoi(*NFText.ToString());

				FString TheirChosenSocketName = FString(TEXT("theirSocket"));
				//int32 TheirReceiverPort = 50012;
				//m_UDPinstance->StartUDPSender(TheirChosenSocketName, m_UDPinstance->TheirReceiverIP, TheirReceiverPort);


				FString TheirReceiverIP = FString(TEXT("127.0.0.1"));
				m_UDPinstance->StartUDPSender(TheirChosenSocketName, TheirReceiverIP, m_UDPinstance->TheirReceiverPort);

				ChatInput->SetHintText(FText::FromString("Type your messages here"));
			}
			else
			{
				m_UDPinstance->UDPSender_SendString(NFText.ToString());
			}

			//FSlateApplication::Get().SetKeyboardFocus(ChatInput);
		}
		ChatInput->SetText(FText()); // clear the chat box now were done with it		

	}

	//FSlateApplication::Get().SetUserFocusToGameViewport(0, EFocusCause::SetDirectly); // set the players focus back to the gameport
}

void SChatWidget::AddMessage(const FSChatMsg& newmessage) 
{
	int32 index = Items.Add(MakeShareable(new FSChatMsg())); 

	if (Items[index].IsValid())
	{
		Items[index]->Init(newmessage.Type, newmessage.Username, newmessage.Text);

		int32 Year, Month, Day, DayOfWeek, Hour, Minute, Second, Millisecond; // set the timestamp and decay timer
		FPlatformTime::SystemTime(Year, Month, DayOfWeek, Day, Hour, Minute, Second, Millisecond);
		Items[index]->SetTime(FText::FromString(FString::Printf(TEXT("[ %02d:%02d:%02d ] "), Hour, Minute, Second)), FPlatformTime::Seconds());

		ListViewWidget->RequestListRefresh(); 
		ListViewWidget->ScrollToBottom(); 
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
