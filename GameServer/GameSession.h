#pragma once
#include "PacketSession.h"

enum : uint16
{
	PKT_CHAT = 1,
};

class GameSession : public PacketSession
{
public:
	GameSession();
	virtual ~GameSession();

protected:
	virtual void	OnRecvPacket(PacketHeader header, const char* buffer, int len) override;
};

