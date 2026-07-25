#include "pch.h"
#include "ChatRoom.h"
#include "ChatSession.h"

ChatRoom::ChatRoom(int roomId)
	: roomId(roomId)
{
}

int ChatRoom::GetUserCount()
{
	std::lock_guard<std::mutex> lock(roomLock);

	for (auto it = sessions.begin(); it != sessions.end(); )
	{
		if (it->second.expired())
			it = sessions.erase(it);
		else
			++it;
	}

	return static_cast<int>(sessions.size());
}

void ChatRoom::Enter(const std::shared_ptr<ChatSession>& session)
{
	if (session == nullptr)
		return;

	std::lock_guard<std::mutex> lock(roomLock);
	sessions[session->GetSessionId()] = session;
}

void ChatRoom::Leave(int sessionId)
{
	std::lock_guard<std::mutex> lock(roomLock);
	sessions.erase(sessionId);
}

void ChatRoom::Broadcast(uint16 packetId, const std::string& payload)
{
	std::vector<std::shared_ptr<ChatSession>> targets;

	{
		std::lock_guard<std::mutex> lock(roomLock);

		for (auto it = sessions.begin(); it != sessions.end(); )
		{
			std::shared_ptr<ChatSession> session = it->second.lock();
			if (session == nullptr)
			{
				it = sessions.erase(it);
				continue;
			}

			targets.push_back(session);
			++it;
		}
	}

	for (auto& session : targets)
	{
		if (session && session->IsConnected())
			session->SendPacket(packetId, payload);
	}
}
