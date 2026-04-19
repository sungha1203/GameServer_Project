#pragma once
#include "PacketSession.h"

extern std::atomic<int> recvCnt;

enum : uint16
{
	PKT_CHAT = 1,
};

class GameSession : public PacketSession
{
public:
	GameSession();
	virtual ~GameSession();

public:
	virtual void	Reset() override;

protected:
	virtual void	OnRecvPacket(PacketHeader header, const char* buffer, int len) override;

private:
	int recvPacketCnt = 0;
};