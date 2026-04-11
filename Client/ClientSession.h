#pragma once
#include "PacketSession.h"
#include "Session.h"

enum : uint16
{
	PKT_CHAT = 1,
};

class ClientSession : public PacketSession
{
public:
	ClientSession();
	virtual ~ClientSession();

public:
	void OnConnected();
	void OnDisconnected();

	bool SendChat(const std::string& msg);

protected:
	virtual void OnRecvPacket(PacketHeader header, const char* buffer, int len) override;

private:
	//ClientSessionManager* clientSessionManager = nullptr;
};

