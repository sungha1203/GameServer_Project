#include "ClientSession.h"

ClientSession::ClientSession()
{
}

ClientSession::~ClientSession()
{
}

void ClientSession::OnConnected()
{
	PLOGI << "연결 완료 : Session ID = " << GetSessionId();
}

void ClientSession::OnDisconnected()
{
	PLOGI << "연결 끊김 : Session ID = " << GetSessionId();
}

bool ClientSession::SendChat(const std::string& msg)
{
	PacketHeader header;
	header.size = static_cast<uint16>(sizeof(PacketHeader) + msg.size());
	header.id = PKT_CHAT;

	std::vector<char> sendBuffer(header.size);
	memcpy(sendBuffer.data(), &header, sizeof(PacketHeader));
	memcpy(sendBuffer.data() + sizeof(PacketHeader), msg.data(), msg.size());

	PLOGD << "ID : " << GetSessionId() << ", 보낸 메시지 : " << msg;

	return Send(sendBuffer.data(), static_cast<int>(sendBuffer.size()));
}

void ClientSession::Reset()
{
	PacketSession::Reset();
}

void ClientSession::OnRecvPacket(PacketHeader header, const char* buffer, int len)
{
	//switch (header.id)
	//{
	//default:
	//	break;
	//}
}