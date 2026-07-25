#pragma once

#include "ChatProtocol.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class ChatSession;

class ChatRoom
{
public:
	ChatRoom(int roomId);

	int GetRoomId() const { return roomId; }
	int GetUserCount();

	void Enter(const std::shared_ptr<ChatSession>& session);
	void Leave(int sessionId);
	void Broadcast(uint16 packetId, const std::string& payload);

private:
	int roomId = 0;
	std::mutex roomLock;
	std::unordered_map<int, std::weak_ptr<ChatSession>> sessions;
};
