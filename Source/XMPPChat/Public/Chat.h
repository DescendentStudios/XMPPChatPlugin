// (c) 2015 Descendent Studios, Inc.

#pragma once

#include "Engine.h"
#include "Xmpp.h"
#include "Chat.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogChat, Warning, All);

/**
* BP Enum EUXmppPresenceStatus mapping to non-BP EXmppPresenceStatus
*/
UENUM(BlueprintType)
namespace EUXmppPresenceStatus
{
	enum Type
	{
		Online,
		Offline,
		Away,
		ExtendedAway,
		DoNotDisturb,
		Chat
	};
}

/**
* BP Enum EUXmppLoginStatus mapping to non-BP EXmppLoginStatus
* Possible XMPP login states 
*/
UENUM(BlueprintType)
namespace EUXmppLoginStatus
{
	enum Type
	{
		LoggedIn,
		LoggedOut
	};
}

/**
* BP Enum EXmppChatMemberRole mapping to non-BP EUChatMemberRole
* Role of a chat room member
*/
UENUM(BlueprintType)
namespace EUChatMemberRole
{
	enum Type
	{
		Owner,
		Moderator,
		Member,
		None,
		Outcast
	};
}

namespace UChatUtil
{
	inline EXmppPresenceStatus::Type GetEXmppPresenceStatus(const EUXmppPresenceStatus::Type Status)
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

	inline EUXmppPresenceStatus::Type GetEUXmppPresenceStatus(const EXmppPresenceStatus::Type Status)
	{
		switch (Status)
		{
		case EXmppPresenceStatus::Online: return EUXmppPresenceStatus::Online;
		case EXmppPresenceStatus::Offline: return EUXmppPresenceStatus::Offline;
		case EXmppPresenceStatus::Away: return EUXmppPresenceStatus::Away;
		case EXmppPresenceStatus::ExtendedAway: return EUXmppPresenceStatus::ExtendedAway;
		case EXmppPresenceStatus::DoNotDisturb: return EUXmppPresenceStatus::DoNotDisturb;
		default:
		case EXmppPresenceStatus::Chat: return EUXmppPresenceStatus::Chat;
		}
	}

	inline EUXmppLoginStatus::Type GetEUXmppLoginStatus(EXmppLoginStatus::Type status)
	{
		switch (status)
		{
		case EXmppLoginStatus::LoggedIn: return EUXmppLoginStatus::LoggedIn;
		default:
		case EXmppLoginStatus::LoggedOut: return EUXmppLoginStatus::LoggedOut;
		}
	}

	inline EUChatMemberRole::Type GetEUChatMemberRole(const EXmppChatMemberRole::Type Status)
	{
		switch (Status)
		{
		case EXmppChatMemberRole::Owner: return EUChatMemberRole::Owner;
		case EXmppChatMemberRole::Moderator: return EUChatMemberRole::Moderator;
		case EXmppChatMemberRole::Member: return EUChatMemberRole::Member;
		case EXmppChatMemberRole::None: return EUChatMemberRole::None;
		default:
		case EXmppChatMemberRole::Outcast: return EUChatMemberRole::Outcast;
		}
	}
}

/** Generate a delegates for callback events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChatLoginComplete, const FString&, UserJid, bool, bWasSuccess, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChatLogoutComplete, const FString&, UserJid, bool, bWasSuccess, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatLogingChanged, const FString&, UserJid, EUXmppLoginStatus::Type, LoginStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChatReceiveMessage, const FString&, UserJid, const FString&, Type, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPrivateChatReceiveMessage, const FString&, UserJid, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMUCReceiveMessage, const FString&, RoomId, const FString&, UserJid, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMUCRoomJoinPublicComplete, bool, bSuccess, const FString&, RoomId, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMUCRoomJoinPrivateComplete, bool, bSuccess, const FString&, RoomId, const FString&, Error);


/**
* BP version of FXmppChatMember
* Member of a chat room
*/
UCLASS(BlueprintType, Blueprintable)
class UChatMember : public UObject
{
public:

	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	FString Nickname;

	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	FString MemberJid;

	/** state of basic online status */
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	TEnumAsByte<EUXmppPresenceStatus::Type> Status;

	/** connected an available to receive messages */
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	bool bIsAvailable;

	/** time when presence was sent by the user */
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	FDateTime SentTime;

	/** client id user is logged in from */
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	FString ClientResource;

	/** string that will be parsed for further displayed presence info */
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	FString StatusStr;
	
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	TEnumAsByte<EUChatMemberRole::Type> Affiliation;

	void ConvertFrom(const FXmppChatMember& ChatMember);
};


/**
* Chat class representing a connection to a chat server
*/
UCLASS(BlueprintType, Blueprintable)
class UChat : public UObject
{
	GENERATED_UCLASS_BODY()

protected:
	TSharedPtr<IXmppConnection> XmppConnection;

	// Map BP Enum EUXmppPresenceStatus mapping to non-BP EXmppPresenceStatus
	EXmppPresenceStatus::Type GetEXmppPresenceStatus(const EUXmppPresenceStatus::Type Status);

	// Map non-BP EXmppLoginStatus to BP Enum EUXmppLoginStatus 
	EUXmppLoginStatus::Type GetEUXmppLoginStatus(EXmppLoginStatus::Type status);

	// has this chat's delegates been set up, etc.
	bool bInited;

	// has this chat been completed?
	bool bDone;

public:
	// Delegates for BP events

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatLoginComplete OnChatLoginComplete;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatLogoutComplete OnChatLogoutComplete;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatLogingChanged OnChatLogingChanged;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatReceiveMessage OnChatReceiveMessage;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnPrivateChatReceiveMessage OnPrivateChatReceiveMessage;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnMUCReceiveMessage OnMUCReceiveMessage;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnMUCRoomJoinPublicComplete OnMUCRoomJoinPublicComplete;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnMUCRoomJoinPrivateComplete OnMUCRoomJoinPrivateComplete;

public:
	// Callbacks for delegates

	void OnLoginCompleteFunc(const FXmppUserJid& UserJid, bool bWasSuccess, const FString& Error);
	void OnLogoutCompleteFunc(const FXmppUserJid& UserJid, bool bWasSuccess, const FString& Error);
	void OnLogingChangedFunc(const FXmppUserJid& UserJid, EXmppLoginStatus::Type LoginStatus);

	void OnChatReceiveMessageFunc(const TSharedRef<IXmppConnection>& Connection, const FXmppUserJid& FromJid, const TSharedRef<FXmppMessage>& Message);
	void OnPrivateChatReceiveMessageFunc(const TSharedRef<IXmppConnection>& Connection, const FXmppUserJid& FromJid, const TSharedRef<FXmppChatMessage>& Message);

	void OnMUCReceiveMessageFunc(const TSharedRef<IXmppConnection>& Connection, const FXmppRoomId& RoomId, const FXmppUserJid& UserJid, const TSharedRef<FXmppChatMessage>& ChatMsg);
	void OnMUCRoomJoinPublicCompleteFunc(const TSharedRef<IXmppConnection>& Connection, bool bSuccess, const FXmppRoomId& RoomId, const FString& Error);
	void OnMUCRoomJoinPrivateCompleteFunc(const TSharedRef<IXmppConnection>& Connection, bool bSuccess, const FXmppRoomId& RoomId, const FString& Error);

protected:
	FDelegateHandle OnLoginCompleteHandle;
	FDelegateHandle OnLogoutCompleteHandle;
	FDelegateHandle OnLogingChangedHandle;
	FDelegateHandle	OnPrivateChatReceiveMessageHandle;
	FDelegateHandle OnChatReceiveMessageHandle;
	FDelegateHandle OnMUCReceiveMessageHandle;
	FDelegateHandle OnMUCRoomJoinPublicCompleteHandle;
	FDelegateHandle OnMUCRoomJoinPrivateCompleteHandle;

protected:
	void Init();
	void DeInit();

public:
	~UChat();

	/***************** Base **************************/

	UFUNCTION(BlueprintCallable, Category = "Chat")
		void Finish();

	/***************** Login/Logout **************************/

	void Login(const FString& UserId, const FString& Auth, const FXmppServer& XmppServer);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void Login(const FString& UserId, const FString& Auth, const FString& ServerAddr, const FString& Domain, const FString& ClientResource);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void Logout();

	/***************** Chat **************************/

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void Message(const FString& UserName, const FString& Recipient, const FString& Type, const FString& MessagePayload);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void PrivateChat(const FString& UserName, const FString& Recipient, const FString& Body);

	/***************** Presence **************************/

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void Presence(bool bIsAvailable, EUXmppPresenceStatus::Type Status, const FString& StatusStr);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void PresenceQuery(const FString& User);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void PresenceGetRosterMembers(TArray<FString>& Members);

	/***************** MUC **************************/

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void MucCreate(const FString& UserName, const FString& RoomId, bool bIsPrivate = false, const FString& Password = "");

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void MucJoin(const FString& RoomId, const FString& Nickname, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void MucExit(const FString& RoomId);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void MucChat(const FString& RoomId, const FString& Body);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void MucConfig(const FString& UserName, const FString& RoomId, bool bIsPrivate, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void MucRefresh(const FString& RoomId);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void MucGetMembers(const FString& RoomId, TArray<UChatMember*>& Members);

	/***************** PubSub **************************/

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void PubSubCreate(const FString& NodeId);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void PubSubDestroy(const FString& NodeId);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void PubSubSubscribe(const FString& NodeId);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void PubSubUnsubscribe(const FString& NodeId);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void PubSubPublish(const FString& NodeId, const FString& Payload);
};