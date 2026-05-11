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
	auto owner = std::move(recvEvent.owner);

	if (!owner) return;

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

	//ProcessPacket();

	//이미 큐에 등록되어 있으면 중복 등록X
	if (packetQueueProcessing.exchange(true) == false)
	{
		if (packetProcessor)
		{
			auto self = std::static_pointer_cast<PacketSession>(owner);
			if (self)
				packetProcessor->Enqueue(self);
		}
	}

	if (!IsConnected())	return;

	// recv 재등록
	RegisterRecv();
}

void PacketSession::Reset()
{
	Session::Reset();

	//packetBufferSize = 0;
	//ZeroMemory(packetBuffer, sizeof(packetBuffer));

	{
		std::lock_guard<std::mutex> lock(packetMutex);
		packetBufferSize = 0;
		ZeroMemory(packetBuffer, sizeof(packetBuffer));
	}

	packetQueueProcessing = false;
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

void PacketSession::ProcessPacket2()
{
	while (true)
	{
		std::vector<char> packet;

		{
			std::lock_guard<std::mutex> lock(packetMutex);

			if (packetBufferSize < sizeof(PacketHeader))
				break;

			PacketHeader header;
			memcpy(&header, packetBuffer, sizeof(PacketHeader));

			if (header.size < sizeof(PacketHeader))
			{
				PLOGE << "잘못된 패킷 크기";
				Disconnect();
				break;
			}

			if (packetBufferSize < header.size)
				break;

			packet.resize(header.size);
			memcpy(packet.data(), packetBuffer, header.size);

			int remainSize = packetBufferSize - header.size;
			memmove(packetBuffer, packetBuffer + header.size, remainSize);
			packetBufferSize = remainSize;
		}

		PacketHeader header;
		memcpy(&header, packet.data(), sizeof(PacketHeader));

		int dataSize = header.size - sizeof(PacketHeader);

		OnRecvPacket(header, packet.data() + sizeof(PacketHeader), dataSize);

		if (IsConnected() == false)
			break;
	}

	packetQueueProcessing.store(false);

	// 처리 도중 새 데이터가 들어왔을 수 있으니 다시 확인
	{
		std::lock_guard<std::mutex> lock(packetMutex);

		if (CompletePacketLocked())
		{
			if (packetQueueProcessing.exchange(true) == false)
			{
				if (packetProcessor)
				{
					auto self = std::static_pointer_cast<PacketSession>(shared_from_this());
					packetProcessor->Enqueue(self);
				}
			}
		}
	}
}

bool PacketSession::CompletePacketLocked()
{
	if (packetBufferSize < sizeof(PacketHeader))
		return false;

	PacketHeader header;
	memcpy(&header, packetBuffer, sizeof(PacketHeader));

	if (header.size < sizeof(PacketHeader))
		return false;

	return packetBufferSize >= header.size;
}
