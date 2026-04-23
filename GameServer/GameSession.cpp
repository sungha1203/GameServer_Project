#include "GameSession.h"

GameSession::GameSession()
{
}

GameSession::~GameSession()
{
}

void GameSession::OnRecvPacket(PacketHeader header, const char* buffer, int len)
{
	switch (header.id)
	{
	case PKT_CHAT:
	{
		int dataSize = header.size - sizeof(PacketHeader);
		string msg(packetBuffer + sizeof(PacketHeader), dataSize);

		++recvCnt;					// 전체 받은 메시지 수
		++recvPacketCnt;			// 10번 받으면 종료
		PLOGD << "ID : " << GetSessionId() << ", 받은 메시지 : " << msg;

		if (recvCnt % 1000 == 0)
			PLOGE << "받은 메시지 수 : " << recvCnt;
		if (recvPacketCnt >= 10)
		{
			Disconnect();
		}
		break;
	}
	default:
		break;
	}
}

void GameSession::Reset()
{
	PacketSession::Reset();
	recvPacketCnt = 0;
}