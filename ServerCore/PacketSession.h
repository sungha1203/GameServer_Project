#pragma once
#include "Session.h"

class PacketSession : public Session // PacketSession은 패킷 조립만 담당
{
public:
	PacketSession();
	virtual ~PacketSession();

public:
	virtual void		ProcessRecv(int numOfBytes) override;
	virtual void		Reset() override;

protected:
	void				ProcessPacket();
	virtual void        OnRecvPacket(PacketHeader header, const char* buffer, int len) = 0;

protected:
	char				packetBuffer[4096]{};
	int					packetBufferSize = 0;
};

