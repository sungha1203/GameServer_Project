#pragma once

#include "ChatRoom.h"

#include <memory>
#include <string>
#include <vector>

class ChatSession;

class ChatRoomManager
{
public:
	void Init(int roomCount);

	bool IsValidRoomId(int roomId) const;
	bool EnterRoom(const std::shared_ptr<ChatSession>& session, int roomId, std::string& reason);
	bool LeaveRoom(const std::shared_ptr<ChatSession>& session, bool notify);
	ChatRoom* GetRoom(int roomId);
	std::string BuildRoomList();

private:
	std::vector<std::unique_ptr<ChatRoom>> rooms;
};
