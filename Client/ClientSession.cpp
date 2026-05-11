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
	string NewMsg = msg + std::to_string(seqNum);
	PacketHeader header;
	header.size = static_cast<uint16>(sizeof(PacketHeader) + NewMsg.size());
	header.id = PKT_CHAT;

	std::vector<char> sendBuffer(header.size);
	memcpy(sendBuffer.data(), &header, sizeof(PacketHeader));
	memcpy(sendBuffer.data() + sizeof(PacketHeader), NewMsg.data(), NewMsg.size());

	++seqNum;

	PLOGD << "ID : " << GetSessionId() << ", 보낸 메시지 : " << NewMsg;

	return Send(sendBuffer.data(), static_cast<int>(sendBuffer.size()));
}

void ClientSession::Reset()
{
	PacketSession::Reset();
	seqNum = 1;
}

void ClientSession::OnRecvPacket(PacketHeader header, const char* buffer, int len)
{
	//switch (header.id)
	//{
	//default:
	//	break;
	//}
}