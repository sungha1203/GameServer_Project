#include "pch.h"
#include "ChatSession.h"
#include "ChatGlobal.h"
#include "ChatRoom.h"
#include "ChatRoomManager.h"
#include "UserManager.h"

ChatSession::ChatSession()
{
}

ChatSession::~ChatSession()
{
}

void ChatSession::OnConnected()
{
	PLOGI << "Chat session connected. Session ID = " << GetSessionId();
}

void ChatSession::OnDisconnected()
{
	PLOGI << "Chat session disconnected. Session ID = " << GetSessionId();

	if (GRoomManager && currentRoomId != 0)
		GRoomManager->LeaveRoom(GetChatSessionPtr(), true);

	if (GUserManager && !nickname.empty())
	{
		GUserManager->Logout(nickname);
		PLOGI << "Logout. nickname=" << nickname << ", users=" << GUserManager->GetUserCount();
	}

	state = ChatSessionState::Connected;
	nickname.clear();
	currentRoomId = 0;
}

void ChatSession::Reset()
{
	PacketSession::Reset();
	state = ChatSessionState::Connected;
	nickname.clear();
	currentRoomId = 0;
}

void ChatSession::OnRecvPacket(PacketHeader header, const char* buffer, int len)
{
	switch (header.id)
	{
	case C_LOGIN_REQ:
		HandleLogin(buffer, len);
		break;
	case C_ROOM_LIST_REQ:
		HandleRoomList(buffer, len);
		break;
	case C_ROOM_ENTER_REQ:
		HandleEnterRoom(buffer, len);
		break;
	case C_ROOM_LEAVE_REQ:
		HandleLeaveRoom(buffer, len);
		break;
	case C_CHAT_REQ:
		HandleChat(buffer, len);
		break;
	default:
		PLOGW << "Invalid packet. Session ID = " << GetSessionId() << ", packetId=" << header.id;
		SendError("Unknown packet id");
		break;
	}
}

void ChatSession::HandleLogin(const char* buffer, int len)
{
	if (state != ChatSessionState::Connected)
	{
		PLOGW << "Login failed. already logged in. Session ID = " << GetSessionId();
		SendPacket(S_LOGIN_RES, "FAIL|Already logged in");
		return;
	}

	if (!GUserManager)
	{
		PLOGE << "Login failed. UserManager is null. Session ID = " << GetSessionId();
		SendPacket(S_LOGIN_RES, "FAIL|Server not ready");
		return;
	}

	std::string assignedNickname;
	std::string reason;
	if (!GUserManager->Login(MakePayload(buffer, len), assignedNickname, reason))
	{
		PLOGW << "Login failed. Session ID = " << GetSessionId() << ", reason=" << reason;
		SendPacket(S_LOGIN_RES, "FAIL|" + reason);
		return;
	}

	nickname = assignedNickname;
	state = ChatSessionState::Authenticated;

	PLOGI << "Login success. Session ID = " << GetSessionId() << ", nickname=" << nickname << ", users=" << GUserManager->GetUserCount();
	SendPacket(S_LOGIN_RES, "OK|" + nickname);

	if (GRoomManager)
		SendPacket(S_ROOM_LIST_RES, GRoomManager->BuildRoomList());
}

void ChatSession::HandleRoomList(const char* buffer, int len)
{
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(len);

	if (state == ChatSessionState::Connected)
	{
		PLOGW << "Room list blocked before login. Session ID = " << GetSessionId();
		SendError("Login required");
		return;
	}

	if (!GRoomManager)
	{
		SendError("Room manager not ready");
		return;
	}

	SendPacket(S_ROOM_LIST_RES, GRoomManager->BuildRoomList());
}

void ChatSession::HandleEnterRoom(const char* buffer, int len)
{
	if (state == ChatSessionState::Connected)
	{
		PLOGW << "Room enter blocked before login. Session ID = " << GetSessionId();
		SendPacket(S_ROOM_ENTER_RES, "FAIL|Login required");
		return;
	}

	if (!GRoomManager)
	{
		SendPacket(S_ROOM_ENTER_RES, "FAIL|Room manager not ready");
		return;
	}

	int roomId = 0;
	try
	{
		roomId = std::stoi(MakePayload(buffer, len));
	}
	catch (...)
	{
		PLOGW << "Room enter failed. invalid room id payload. nickname=" << nickname;
		SendPacket(S_ROOM_ENTER_RES, "FAIL|Invalid room id");
		return;
	}

	std::string reason;
	if (!GRoomManager->EnterRoom(GetChatSessionPtr(), roomId, reason))
	{
		PLOGW << "Room enter failed. nickname=" << nickname << ", roomId=" << roomId << ", reason=" << reason;
		SendPacket(S_ROOM_ENTER_RES, "FAIL|" + reason);
		return;
	}

	state = ChatSessionState::InRoom;

	ChatRoom* room = GRoomManager->GetRoom(roomId);
	const int userCount = room ? room->GetUserCount() : 0;
	SendPacket(S_ROOM_ENTER_RES, "OK|" + std::to_string(roomId) + "|" + std::to_string(userCount));

	if (room)
	{
		const std::string joinPayload = nickname + "|" + std::to_string(roomId) + "|" + std::to_string(userCount);
		room->Broadcast(S_ROOM_USER_JOIN, joinPayload);
	}
}

void ChatSession::HandleLeaveRoom(const char* buffer, int len)
{
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(len);

	if (state != ChatSessionState::InRoom || currentRoomId == 0)
	{
		PLOGW << "Room leave blocked. not in room. Session ID = " << GetSessionId();
		SendPacket(S_ROOM_LEAVE_RES, "FAIL|Not in room");
		return;
	}

	const int roomId = currentRoomId;
	if (GRoomManager && GRoomManager->LeaveRoom(GetChatSessionPtr(), true))
	{
		state = ChatSessionState::Authenticated;
		SendPacket(S_ROOM_LEAVE_RES, "OK|" + std::to_string(roomId));
		return;
	}

	SendPacket(S_ROOM_LEAVE_RES, "FAIL|Leave failed");
}

void ChatSession::HandleChat(const char* buffer, int len)
{
	if (state == ChatSessionState::Connected)
	{
		PLOGW << "Chat blocked before login. Session ID = " << GetSessionId();
		SendError("Login required");
		return;
	}

	if (state != ChatSessionState::InRoom || currentRoomId == 0)
	{
		PLOGW << "Chat blocked before room enter. nickname=" << nickname;
		SendError("Enter room first");
		return;
	}

	if (!GRoomManager)
	{
		SendError("Room manager not ready");
		return;
	}

	ChatRoom* room = GRoomManager->GetRoom(currentRoomId);
	if (!room)
	{
		PLOGW << "Chat failed. invalid current room. nickname=" << nickname << ", roomId=" << currentRoomId;
		SendError("Invalid room");
		return;
	}

	std::string message = MakePayload(buffer, len);
	if (message.empty())
	{
		SendError("Empty chat message");
		return;
	}

	room->Broadcast(S_CHAT, nickname + "|" + message);
	PLOGD << "Chat broadcast. roomId=" << currentRoomId << ", nickname=" << nickname << ", bytes=" << message.size();
}

void ChatSession::SendPacket(uint16 packetId, const std::string& payload)
{
	PacketHeader header;
	header.size = static_cast<uint16>(sizeof(PacketHeader) + payload.size());
	header.id = packetId;

	std::vector<char> sendBuffer(header.size);
	memcpy(sendBuffer.data(), &header, sizeof(PacketHeader));
	memcpy(sendBuffer.data() + sizeof(PacketHeader), payload.data(), payload.size());

	Send(sendBuffer.data(), static_cast<int>(sendBuffer.size()));
}

void ChatSession::SendError(const std::string& message)
{
	SendPacket(S_ERROR, message);
}

std::shared_ptr<ChatSession> ChatSession::GetChatSessionPtr()
{
	return std::static_pointer_cast<ChatSession>(shared_from_this());
}

std::string ChatSession::MakePayload(const char* buffer, int len)
{
	if (buffer == nullptr || len <= 0)
		return "";

	return std::string(buffer, buffer + len);
}
