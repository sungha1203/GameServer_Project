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

		PLOGD << "Recv Packet : " << msg << endl;
		//PLOGD << "받은 패킷 수 : " << ++g_recvCnt;
		break;
	}
	default:
		break;
	}
}
