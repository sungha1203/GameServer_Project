#pragma once

#include "PacketSession.h"
#include "ChatProtocol.h"

enum class ChatSessionState
{
	Connected,
	Authenticated,
	InRoom,
};

class ChatSession : public PacketSession
{
public:
	ChatSession();
	virtual ~ChatSession();

public:
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void Reset() override;

	void SendPacket(uint16 packetId, const std::string& payload);
	const std::string& GetNickname() const { return nickname; }
	int GetCurrentRoomId() const { return currentRoomId; }
	void SetCurrentRoomId(int roomId) { currentRoomId = roomId; }

protected:
	virtual void OnRecvPacket(PacketHeader header, const char* buffer, int len) override;

private:
	void HandleLogin(const char* buffer, int len);
	void HandleRoomList(const char* buffer, int len);
	void HandleEnterRoom(const char* buffer, int len);
	void HandleLeaveRoom(const char* buffer, int len);
	void HandleChat(const char* buffer, int len);
	void SendError(const std::string& message);
	std::shared_ptr<ChatSession> GetChatSessionPtr();
	std::string MakePayload(const char* buffer, int len);

private:
	ChatSessionState state = ChatSessionState::Connected;
	std::string nickname;
	int currentRoomId = 0;
};
