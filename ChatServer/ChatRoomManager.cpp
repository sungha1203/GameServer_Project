#include "pch.h"
#include "ChatRoomManager.h"
#include "ChatProtocol.h"
#include "ChatRoom.h"
#include "ChatSession.h"

void ChatRoomManager::Init(int roomCount)
{
	rooms.clear();
	rooms.reserve(roomCount + 1);
	rooms.push_back(nullptr);

	for (int roomId = 1; roomId <= roomCount; ++roomId)
		rooms.push_back(std::make_unique<ChatRoom>(roomId));
}

bool ChatRoomManager::IsValidRoomId(int roomId) const
{
	return roomId > 0 && roomId < static_cast<int>(rooms.size()) && rooms[roomId] != nullptr;
}

bool ChatRoomManager::EnterRoom(const std::shared_ptr<ChatSession>& session, int roomId, std::string& reason)
{
	if (session == nullptr)
	{
		reason = "Invalid session";
		return false;
	}

	if (!IsValidRoomId(roomId))
	{
		reason = "Invalid room id";
		return false;
	}

	if (session->GetCurrentRoomId() == roomId)
	{
		reason = "Already in room";
		return false;
	}

	if (session->GetCurrentRoomId() != 0)
		LeaveRoom(session, true);

	ChatRoom* room = GetRoom(roomId);
	room->Enter(session);
	session->SetCurrentRoomId(roomId);

	const int userCount = room->GetUserCount();
	PLOGI << "Room enter. roomId=" << roomId << ", nickname=" << session->GetNickname() << ", users=" << userCount;
	return true;
}

bool ChatRoomManager::LeaveRoom(const std::shared_ptr<ChatSession>& session, bool notify)
{
	if (session == nullptr)
		return false;

	const int roomId = session->GetCurrentRoomId();
	if (!IsValidRoomId(roomId))
		return false;

	ChatRoom* room = GetRoom(roomId);
	const std::string nickname = session->GetNickname();
	const int sessionId = session->GetSessionId();

	room->Leave(sessionId);
	session->SetCurrentRoomId(0);

	const int userCount = room->GetUserCount();
	if (notify)
	{
		const std::string payload = nickname + "|" + std::to_string(roomId) + "|" + std::to_string(userCount);
		room->Broadcast(S_ROOM_USER_LEAVE, payload);
	}

	PLOGI << "Room leave. roomId=" << roomId << ", nickname=" << nickname << ", users=" << userCount;
	return true;
}

ChatRoom* ChatRoomManager::GetRoom(int roomId)
{
	if (!IsValidRoomId(roomId))
		return nullptr;

	return rooms[roomId].get();
}

std::string ChatRoomManager::BuildRoomList()
{
	std::string payload;

	for (int roomId = 1; roomId < static_cast<int>(rooms.size()); ++roomId)
	{
		if (!payload.empty())
			payload += "|";

		payload += std::to_string(roomId);
		payload += ":";
		payload += std::to_string(rooms[roomId]->GetUserCount());
	}

	return payload;
}
