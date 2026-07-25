#pragma once

#include "ChatProtocol.h"
#include "PacketSession.h"

class ChatClientSession : public PacketSession
{
public:
	ChatClientSession();
	virtual ~ChatClientSession();

	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void Reset() override;

	bool SendLogin(const std::string& nickname);
	bool SendRoomListReq();
	bool SendRoomEnter(int roomId);
	bool SendRoomLeave();
	bool SendChat(const std::string& message);

protected:
	virtual void OnRecvPacket(PacketHeader header, const char* buffer, int len) override;

private:
	bool SendPacket(uint16 packetId, const std::string& payload);
	std::string MakePayload(const char* buffer, int len);
	void PrintPacket(uint16 packetId, const std::string& payload);
};
