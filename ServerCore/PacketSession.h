#pragma once
#include "Session.h"
#include "PacketProcessor.h"
#include "CoreGlobal.h"

class PacketSession : public Session //, public std::enable_shared_from_this<Session>// PacketSession은 패킷 조립만 담당
{
public:
	PacketSession();
	virtual ~PacketSession();

public:
	virtual void		ProcessRecv(int numOfBytes) override;
	virtual void		Reset() override;
	void				ProcessPacket2();				// new 

protected:
	void				ProcessPacket();
	virtual void        OnRecvPacket(PacketHeader header, const char* buffer, int len) = 0;

protected:
	char				packetBuffer[4096]{};
	int					packetBufferSize = 0;

private:
	bool				CompletePacketLocked();			// 패킷 버퍼 안에 완성된 패킷이 있는지 확인

	std::mutex			packetMutex;					// 여러 스레드에서 접근할 수 있으므로 보호
	std::atomic<bool>	packetQueueProcessing{ false };	// 패킷 처리 중인지 여부(PacketProcessor 큐에 중복으로 여러번 들어오는걸 막기 위해)
};