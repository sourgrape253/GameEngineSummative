// Fill out your copyright notice in the Description page of Project Settings.

#include "SChatWidget.h"
#include "SlateOptMacros.h"
#include "chatPlugin.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SChatWidget::Construct(const FArguments& InArgs)
{
	FText hint = FText::FromString("Do you want to run a client or server (C/S)?");
	if (m_UDPinstance)
	{
		if (!m_UDPinstance->bIsServerClientChosen)
		{
			hint = FText::FromString("Do you want to run a client or server (C/S)?");
		}
		else
		{
			hint = FText::FromString("Please type here");
		}
	}	

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
				.HintText(hint)
			]
		]
	];
}

//SChatWidget::~SChatWidget()
//{
//	UE_LOG(LogTemp, Warning, TEXT("Your message"));
//
//	//m_UDPinstance->UDPSender_SendString(FString(TEXT("closing closing")));
//}


TSharedRef<ITableRow> SChatWidget::OnGenerateRowForList(TSharedPtr< FSChatMsg > Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!Items.IsValidIndex(0) || !Item.IsValid() || !Item.Get()) 
		return
		SNew(STableRow< TSharedPtr< FSChatMsg > >, OwnerTable)
		[
			SNew(SBox)
		];

	if (Item.Get()->Type == 1) 
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
				.Text(Item.Get()->Username)
				.ColorAndOpacity(FLinearColor::White)
				.ShadowColorAndOpacity(FLinearColor::Black)
				.ShadowOffset(FIntPoint(1, 1))
				]
			+ SWrapBox::Slot()
			[
				SNew(STextBlock) 
				.Text(FText::FromString(" :  "))
				.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.f))
				.ShadowColorAndOpacity(FLinearColor::Black)
				.ShadowOffset(FIntPoint(1, 1))
			]
			+ SWrapBox::Slot()
			[
				SNew(STextBlock)
				.Text(Item.Get()->Text)
				.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.f))
				.ShadowColorAndOpacity(FLinearColor::Black)
				.ShadowOffset(FIntPoint(1, 1))
			]
		];
	else
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
			if (!m_UDPinstance->m_bServer)
			{
				FSChatMsg newmessage;
				newmessage.Init(1, FText::FromString("Me"), NFText);
				AddMessage(newmessage);
			}

			if (!m_UDPinstance->m_bConnected)
			{
				if (!m_UDPinstance->bIsServerClientChosen)
				{
					if (*NFText.ToString() == FString(TEXT("S")) || *NFText.ToString() == FString(TEXT("s")) )
					{
						m_UDPinstance->m_bServer = true;

						FString MyChosenSocketName = FString(TEXT("mySocket"));
						int32 MyReceivingPort = DEFAULT_SERVER_PORT;

						if (m_UDPinstance->StartUDPReceiver(MyChosenSocketName, m_UDPinstance->MyReceivingIP, MyReceivingPort))
						{
							m_UDPinstance->CreateSenderSocket(FString(TEXT("theirSocket")));

							ChatInput->SetHintText(FText::FromString("I am the server. Please do not type in here."));

							m_UDPinstance->m_myName = FString(TEXT("Server"));
						}
						else
						{
							ChatInput->SetHintText(FText::FromString("Something wrong with the server"));
						}

					}
					else
					{
						ChatInput->SetHintText(FText::FromString("What is your name? "));
						
						ChatInput->SetClearKeyboardFocusOnCommit(false);
					}
					m_UDPinstance->bIsServerClientChosen = true;
				}
				else if (!m_UDPinstance->m_bServer)
				{
					if (!m_UDPinstance->bNameAdded)
					{
						m_UDPinstance->m_myName = NFText.ToString();

						ChatInput->SetHintText(FText::FromString("Please enter your unique port number (Suggested value 60012 - 60022): "));

						m_UDPinstance->bNameAdded = true;
					}
					else if (!m_UDPinstance->bPortAdded)
					{
						m_UDPinstance->MyReceivingPort = FCString::Atoi(*NFText.ToString());

						FString MyChosenSocketName = FString(TEXT("mySocket"));

						if (m_UDPinstance->MyReceivingPort == 0)
						{
							ChatInput->SetHintText(FText::FromString("Please enter your UNIQUE port number (Suggested value 60012 - 60022): "));
						}
						else
						{
							if (m_UDPinstance->StartUDPReceiver(MyChosenSocketName, m_UDPinstance->MyReceivingIP, m_UDPinstance->MyReceivingPort))
							{
								ChatInput->SetHintText(FText::FromString("Please enter the Server's IP address: "));

								m_UDPinstance->bPortAdded = true;
							}
							else
							{
								ChatInput->SetHintText(FText::FromString("Please enter a different UNIQUE port number (Suggested value 60012 - 60022): "));
							}
						}

					}
					else if (!m_UDPinstance->bServerIPAdded)
					{
						FString TheirReceiverIP = NFText.ToString();

						FString TheirChosenSocketName = FString(TEXT("theirSocket"));
						int32 TheirReceiverPort = DEFAULT_SERVER_PORT;

						if (m_UDPinstance->StartUDPSender(TheirChosenSocketName, TheirReceiverIP, TheirReceiverPort))
						{
							m_UDPinstance->UDPSender_SendString(m_UDPinstance->m_myName);

							ChatInput->SetHintText(FText::FromString("Type your messages here"));

							m_UDPinstance->m_bConnected = true;

							m_UDPinstance->bServerIPAdded = true;
						}
						else
						{
							ChatInput->SetHintText(FText::FromString("Please enter the CORRECT Server's IP address: "));
						}
					}
				}
			}
			else if(!m_UDPinstance->m_bServer)
			{
				m_UDPinstance->UDPSender_SendString(NFText.ToString());
			}			
		}
		ChatInput->SetText(FText());
	}
}

void SChatWidget::AddMessage(const FSChatMsg& newmessage) 
{
	int32 index = Items.Add(MakeShareable(new FSChatMsg())); 

	if (Items[index].IsValid())
	{
		Items[index]->Init(newmessage.Type, newmessage.Username, newmessage.Text);

		int32 Year, Month, Day, DayOfWeek, Hour, Minute, Second, Millisecond;
		FPlatformTime::SystemTime(Year, Month, DayOfWeek, Day, Hour, Minute, Second, Millisecond);
		Items[index]->SetTime(FText::FromString(FString::Printf(TEXT("[ %02d:%02d:%02d ] "), Hour, Minute, Second)), FPlatformTime::Seconds());

		ListViewWidget->RequestListRefresh(); 
		ListViewWidget->ScrollToBottom(); 
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
