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
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;

	bool SendChat(const std::string& msg);
	virtual void Reset() override;

protected:
	virtual void OnRecvPacket(PacketHeader header, const char* buffer, int len) override;

private:
	int               seqNum = 1;
	//SessionManager* clientSessionManager = nullptr;
};

