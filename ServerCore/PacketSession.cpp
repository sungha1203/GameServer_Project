#include "pch.h"
#include "PacketSession.h"

PacketSession::PacketSession()
{
}

PacketSession::~PacketSession()
{
}

void PacketSession::ProcessRecv(int numOfBytes)
{
	if (numOfBytes == 0)
	{
		if (socket != INVALID_SOCKET)
		{
			Disconnect();
			return;
		}
	}
	if (packetBufferSize + numOfBytes > sizeof(packetBuffer))
	{
		Disconnect();
		return;
	}

	memcpy(packetBuffer + packetBufferSize, recvEvent.buffer, numOfBytes);
	packetBufferSize += numOfBytes;

	ProcessPacket();

	if (!IsConnected())	return;

	// recv 재등록
	RegisterRecv();
}

void PacketSession::Reset()
{
	Session::Reset();

	packetBufferSize = 0;
	ZeroMemory(packetBuffer, sizeof(packetBuffer));
}

void PacketSession::ProcessPacket()
{
	while (1)
	{
		// 헤더 4바이트가 다 모일때까지 대기
		if (packetBufferSize < sizeof(PacketHeader))
			return;

		PacketHeader header;
		memcpy(&header, packetBuffer, sizeof(PacketHeader));

		if (header.size < sizeof(PacketHeader))
		{
			PLOGE << "잘못된 패킷 크기";
			Disconnect();
			return;
		}

		if (packetBufferSize < header.size)
			return;

		int dataSize = header.size - sizeof(PacketHeader);
		OnRecvPacket(header, packetBuffer + sizeof(PacketHeader), dataSize);

		// OnRecvPacket에서 Disconnect()가 호출됐을 수 있으므로 연결 상태 확인
		if (!isConnected)	return;

		int remainSize = packetBufferSize - header.size;
		memmove(packetBuffer, packetBuffer + header.size, remainSize);
		packetBufferSize = remainSize;
	}
}