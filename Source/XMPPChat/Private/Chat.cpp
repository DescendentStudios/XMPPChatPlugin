// (c) 2015 Descendent Studios, Inc.

#include "XMPPChatPrivatePCH.h"
#include "Chat.h"

#include "ModuleManager.h"
#include "Xmpp.h"
#include "XmppConnection.h"

DEFINE_LOG_CATEGORY(LogChat);

UChat::UChat(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), bInited(false), bDone(false)
{
}

UChat::~UChat()
{
	DeInit();
}

void UChat::Init()
{
	if (XmppConnection.IsValid() && !bInited)
	{
		bInited = true;

		IXmppConnection::FOnXmppLoginComplete& OnXMPPLoginCompleteDelegate = XmppConnection->OnLoginComplete();
		OnLoginCompleteHandle = OnXMPPLoginCompleteDelegate.AddUObject(this, &UChat::OnLoginCompleteFunc);

		IXmppConnection::FOnXmppLogoutComplete& OnXMPPLogoutCompleteDelegate = XmppConnection->OnLogoutComplete();
		OnLogoutCompleteHandle = OnXMPPLogoutCompleteDelegate.AddUObject(this, &UChat::OnLogoutCompleteFunc);

		IXmppConnection::FOnXmppLogingChanged& OnXMPPLogingChangedDelegate = XmppConnection->OnLoginChanged();
		OnLogingChangedHandle = OnXMPPLogingChangedDelegate.AddUObject(this, &UChat::OnLogingChangedFunc);

		IXmppMessages::FOnXmppMessageReceived& OnXMPPReceiveMessageDelegate = XmppConnection->Messages()->OnReceiveMessage();
		OnChatReceiveMessageHandle = OnXMPPReceiveMessageDelegate.AddUObject(this, &UChat::OnChatReceiveMessageFunc);

		IXmppChat::FOnXmppChatReceived& OnXMPPChatReceivedDelegate = XmppConnection->PrivateChat()->OnReceiveChat();
		OnPrivateChatReceiveMessageHandle = OnXMPPChatReceivedDelegate.AddUObject(this, &UChat::OnPrivateChatReceiveMessageFunc);

		IXmppMultiUserChat::FOnXmppRoomChatReceived& OnXMPPMUCReceiveMessageDelegate = XmppConnection->MultiUserChat()->OnRoomChatReceived();
		OnMUCReceiveMessageHandle = OnXMPPMUCReceiveMessageDelegate.AddUObject(this, &UChat::OnMUCReceiveMessageFunc);

		IXmppMultiUserChat::FOnXmppRoomJoinPublicComplete& OnXMPPMUCRoomJoinPublicDelegate = XmppConnection->MultiUserChat()->OnJoinPublicRoom();
		OnMUCRoomJoinPublicCompleteHandle = OnXMPPMUCRoomJoinPublicDelegate.AddUObject(this, &UChat::OnMUCRoomJoinPublicCompleteFunc);

		IXmppMultiUserChat::FOnXmppRoomJoinPrivateComplete& OnXMPPMUCRoomJoinPrivateDelegate = XmppConnection->MultiUserChat()->OnJoinPrivateRoom();
		OnMUCRoomJoinPrivateCompleteHandle = OnXMPPMUCRoomJoinPrivateDelegate.AddUObject(this, &UChat::OnMUCRoomJoinPrivateCompleteFunc);
	}
}

void UChat::DeInit()
{
	if (XmppConnection.IsValid())
	{
		bInited = false;

		XmppConnection->OnLoginComplete().Remove(OnLoginCompleteHandle);
		XmppConnection->OnLogoutComplete().Remove(OnLogoutCompleteHandle);
		XmppConnection->OnLoginChanged().Remove(OnLogingChangedHandle);
		XmppConnection->Messages()->OnReceiveMessage().Remove(OnChatReceiveMessageHandle);
		XmppConnection->PrivateChat()->OnReceiveChat().Remove(OnPrivateChatReceiveMessageHandle);
		XmppConnection->MultiUserChat()->OnRoomChatReceived().Remove(OnMUCReceiveMessageHandle);
		XmppConnection->MultiUserChat()->OnRoomChatReceived().Remove(OnMUCRoomJoinPublicCompleteHandle);
		XmppConnection->MultiUserChat()->OnRoomChatReceived().Remove(OnMUCRoomJoinPrivateCompleteHandle);

		FXmppModule::Get().RemoveConnection(XmppConnection.ToSharedRef());
	}	
}

void UChat::Login(const FString& UserId, const FString& Auth, const FString& ServerAddr, const FString& Domain, const FString& ClientResource)
{
	FXmppServer XmppServer;
	XmppServer.ServerAddr = ServerAddr;
	XmppServer.Domain = Domain;
	XmppServer.ClientResource = ClientResource;

	Login(UserId, Auth, XmppServer);
}

void UChat::Login(const FString& UserId, const FString& Auth, const FXmppServer& XmppServer)
{
	FXmppModule& Module = FModuleManager::GetModuleChecked<FXmppModule>("XMPP");

	XmppConnection = Module.CreateConnection(UserId);

	if (XmppConnection.IsValid())
	{
		Init();

		XmppConnection->SetServer(XmppServer);

		XmppConnection->Login(UserId, Auth);
	}
}


void UChat::OnLoginCompleteFunc(const FXmppUserJid& UserJid, bool bWasSuccess, const FString& Error)
{
	UE_LOG(LogChat, Log, TEXT("UChat::OnLoginComplete UserJid=%s Success=%s Error=%s"),	*UserJid.GetFullPath(), bWasSuccess ? TEXT("true") : TEXT("false"), *Error);

	OnChatLoginComplete.Broadcast(UserJid.GetFullPath(), bWasSuccess, Error);
}

void UChat::OnLogoutCompleteFunc(const FXmppUserJid& UserJid, bool bWasSuccess, const FString& Error)
{
	UE_LOG(LogChat, Log, TEXT("UChat::OnLogoutComplete UserJid=%s Success=%s Error=%s"), *UserJid.GetFullPath(), bWasSuccess ? TEXT("true") : TEXT("false"), *Error);	

	OnChatLogoutComplete.Broadcast(UserJid.GetFullPath(), bWasSuccess, Error);
}

void UChat::OnLogingChangedFunc(const FXmppUserJid& UserJid, EXmppLoginStatus::Type LoginStatus)
{
	UE_LOG(LogChat, Log, TEXT("UChat::OnLogingChanged UserJid=%s LoginStatus=%d"), *UserJid.GetFullPath(), static_cast<int32>(LoginStatus));

	OnChatLogingChanged.Broadcast(UserJid.GetFullPath(), GetEUXmppLoginStatus(LoginStatus));
}

void UChat::OnChatReceiveMessageFunc(const TSharedRef<IXmppConnection>& Connection, const FXmppUserJid& FromJid, const TSharedRef<FXmppMessage>& Message)
{
	UE_LOG(LogChat, Log, TEXT("UChat::OnChatReceiveMessage UserJid=%s Message=%s"), *FromJid.GetFullPath(), *Message->Payload);

	OnChatReceiveMessage.Broadcast(FromJid.GetFullPath(), Message->Payload);
}

void UChat::OnPrivateChatReceiveMessageFunc(const TSharedRef<IXmppConnection>& Connection, const FXmppUserJid& FromJid, const TSharedRef<FXmppChatMessage>& Message)
{
	UE_LOG(LogChat, Log, TEXT("UChat::OnPrivateChatReceiveMessage UserJid=%s Message=%s"), *FromJid.GetFullPath(), *Message->Body);

	OnPrivateChatReceiveMessage.Broadcast(FromJid.GetFullPath(), Message->Body);
}

void UChat::OnMUCReceiveMessageFunc(const TSharedRef<IXmppConnection>& Connection, const FXmppRoomId& RoomId, const FXmppUserJid& UserJid, const TSharedRef<FXmppChatMessage>& ChatMsg)
{
	TSharedPtr<FXmppChatMember> messageSender = Connection->MultiUserChat()->GetMember(RoomId, UserJid);
	FString displayName = "Unknown User";
	if (messageSender.IsValid())
	{
		displayName = messageSender->Nickname;
	}
	OnMUCReceiveMessage.Broadcast(static_cast<FString>(RoomId), displayName, *ChatMsg->Body);
}

void UChat::OnMUCRoomJoinPublicCompleteFunc(const TSharedRef<IXmppConnection>& Connection, bool bSuccess, const FXmppRoomId& RoomId, const FString& Error)
{
	OnMUCRoomJoinPublicComplete.Broadcast(bSuccess, static_cast<FString>(RoomId), Error);
}

void UChat::OnMUCRoomJoinPrivateCompleteFunc(const TSharedRef<IXmppConnection>& Connection, bool bSuccess, const FXmppRoomId& RoomId, const FString& Error)
{
	OnMUCRoomJoinPrivateComplete.Broadcast(bSuccess, static_cast<FString>(RoomId), Error);
}

void UChat::Finish()
{	
	if (XmppConnection.IsValid())
	{
		if (XmppConnection->GetLoginStatus() == EXmppLoginStatus::LoggedIn)
		{
			Logout();
		}
		else
		{
			bDone = true;
		}
	}
}

void UChat::Presence(bool bIsAvailable, EUXmppPresenceStatus::Type Status, const FString& StatusStr)
{
	if (XmppConnection->Presence().IsValid())
	{		
		FXmppUserPresence XmppPresence = XmppConnection->Presence()->GetPresence();
		XmppPresence.bIsAvailable = bIsAvailable;
		XmppPresence.Status = GetEXmppPresenceStatus(Status);
		XmppConnection->Presence()->UpdatePresence(XmppPresence);
	}
}

void UChat::PresenceQuery(const FString& User)
{
	if (XmppConnection->Presence().IsValid())
	{
		XmppConnection->Presence()->QueryPresence(User);
	}
}

void UChat::Message(const FString& UserName, const FString& Recipient, const FString& MessagePayload)
{
	if (XmppConnection->Messages().IsValid())
	{		
		FXmppMessage Message;
		Message.FromJid.Id = UserName;
		Message.ToJid.Id = Recipient;
		Message.Type = TEXT("test");
		Message.Payload = MessagePayload;
		XmppConnection->Messages()->SendMessage(Recipient, Message);
	}
}

void UChat::PrivateChat(const FString& UserName, const FString& Recipient, const FString& Body)
{
	if (XmppConnection->PrivateChat().IsValid())
	{
		FXmppChatMessage ChatMessage;
		ChatMessage.FromJid.Id = UserName;
		ChatMessage.ToJid.Id = Recipient;
		ChatMessage.Body = Body;
		XmppConnection->PrivateChat()->SendChat(Recipient, ChatMessage);
	}
}

void UChat::Logout()
{
	if (XmppConnection.IsValid() && (XmppConnection->GetLoginStatus() == EXmppLoginStatus::LoggedIn))
	{
		XmppConnection->Logout();		
	}
}

void UChat::MucCreate(const FString& UserName, const FString& RoomId, bool bIsPrivate, const FString& Password)
{
	if (XmppConnection.IsValid() && XmppConnection->MultiUserChat().IsValid())
	{
		FXmppRoomConfig RoomConfig;
		RoomConfig.RoomName = RoomId;		
		RoomConfig.bIsPersistent = false;
		RoomConfig.bIsPrivate = bIsPrivate;		
		RoomConfig.Password = Password;
		XmppConnection->MultiUserChat()->CreateRoom(RoomId, UserName, RoomConfig);
	}
}

void UChat::MucJoin(const FString& RoomId, const FString& Nickname, const FString& Password)
{
	if (XmppConnection.IsValid() && XmppConnection->MultiUserChat().IsValid())
	{
		if (Password.IsEmpty())
		{
			XmppConnection->MultiUserChat()->JoinPublicRoom(RoomId, Nickname);
		}
		else
		{
			XmppConnection->MultiUserChat()->JoinPrivateRoom(RoomId, Nickname, Password);
		}
	}
}

void UChat::MucExit(const FString& RoomId)
{
	if (XmppConnection.IsValid() && XmppConnection->MultiUserChat().IsValid())
	{
		XmppConnection->MultiUserChat()->ExitRoom(RoomId);
	}
}

void UChat::MucChat(const FString& RoomId, const FString& Body)
{				
	if (XmppConnection.IsValid() && XmppConnection->MultiUserChat().IsValid())
	{
		XmppConnection->MultiUserChat()->SendChat(RoomId, Body);
	}
}

void UChat::MucConfig(const FString& UserName, const FString& RoomId, bool bIsPrivate, const FString& Password)
{
	if (XmppConnection.IsValid() && XmppConnection->MultiUserChat().IsValid())
	{
		FXmppRoomConfig RoomConfig;
		RoomConfig.bIsPrivate = bIsPrivate;
		RoomConfig.Password = Password;
		XmppConnection->MultiUserChat()->ConfigureRoom(RoomId, RoomConfig);
	}
}

void UChat::MucRefresh(const FString& RoomId)
{
	if (XmppConnection.IsValid() && XmppConnection->MultiUserChat().IsValid())
	{
		XmppConnection->MultiUserChat()->RefreshRoomInfo(RoomId);
	}
}

void UChat::PubSubCreate(const FString& NodeId)
{
	if (XmppConnection.IsValid() && XmppConnection->PubSub().IsValid())
	{
		FXmppPubSubConfig PubSubConfig;
		XmppConnection->PubSub()->CreateNode(NodeId, PubSubConfig);
	}
}

void UChat::PubSubDestroy(const FString& NodeId)
{
	if (XmppConnection.IsValid() && XmppConnection->PubSub().IsValid())
	{
		XmppConnection->PubSub()->DestroyNode(NodeId);
	}
}

void UChat::PubSubSubscribe(const FString& NodeId)
{
	if (XmppConnection.IsValid() && XmppConnection->PubSub().IsValid())
	{
		XmppConnection->PubSub()->Subscribe(NodeId);
	}
}

void UChat::PubSubUnsubscribe(const FString& NodeId)
{
	if (XmppConnection.IsValid() && XmppConnection->PubSub().IsValid())
	{
		XmppConnection->PubSub()->Unsubscribe(NodeId);
	}
}

void UChat::PubSubPublish(const FString& NodeId, const FString& Payload)
{
	if (XmppConnection.IsValid() && XmppConnection->PubSub().IsValid())
	{
		FXmppPubSubMessage Message;
		Message.Payload = Payload;
		XmppConnection->PubSub()->PublishMessage(NodeId, Message);
	}
}

EXmppPresenceStatus::Type UChat::GetEXmppPresenceStatus(const EUXmppPresenceStatus::Type Status)
{
	switch (Status)
	{
	case EUXmppPresenceStatus::Online: return EXmppPresenceStatus::Online;
	case EUXmppPresenceStatus::Offline: return EXmppPresenceStatus::Offline;
	case EUXmppPresenceStatus::Away: return EXmppPresenceStatus::Away;
	case EUXmppPresenceStatus::ExtendedAway: return EXmppPresenceStatus::ExtendedAway;
	case EUXmppPresenceStatus::DoNotDisturb: return EXmppPresenceStatus::DoNotDisturb;
	default:
	case EUXmppPresenceStatus::Chat: return EXmppPresenceStatus::Chat;
	}
}

EUXmppLoginStatus::Type UChat::GetEUXmppLoginStatus(EXmppLoginStatus::Type status)
{
	switch (status)
	{
	case EXmppLoginStatus::LoggedIn: return EUXmppLoginStatus::LoggedIn;
	default:
	case EXmppLoginStatus::LoggedOut: return EUXmppLoginStatus::LoggedOut;
	}
}



