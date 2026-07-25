#include "pch.h"
#include "ChatClientSession.h"

#include <iostream>

ChatClientSession::ChatClientSession()
{
}

ChatClientSession::~ChatClientSession()
{
}

void ChatClientSession::OnConnected()
{
	PLOGI << "Connected. Session ID = " << GetSessionId();
}

void ChatClientSession::OnDisconnected()
{
	PLOGI << "Disconnected. Session ID = " << GetSessionId();
}

void ChatClientSession::Reset()
{
	PacketSession::Reset();
}

bool ChatClientSession::SendLogin(const std::string& nickname)
{
	return SendPacket(C_LOGIN_REQ, nickname);
}

bool ChatClientSession::SendRoomListReq()
{
	return SendPacket(C_ROOM_LIST_REQ, "");
}

bool ChatClientSession::SendRoomEnter(int roomId)
{
	return SendPacket(C_ROOM_ENTER_REQ, std::to_string(roomId));
}

bool ChatClientSession::SendRoomLeave()
{
	return SendPacket(C_ROOM_LEAVE_REQ, "");
}

bool ChatClientSession::SendChat(const std::string& message)
{
	return SendPacket(C_CHAT_REQ, message);
}

void ChatClientSession::OnRecvPacket(PacketHeader header, const char* buffer, int len)
{
	PrintPacket(header.id, MakePayload(buffer, len));
}

bool ChatClientSession::SendPacket(uint16 packetId, const std::string& payload)
{
	PacketHeader header;
	header.size = static_cast<uint16>(sizeof(PacketHeader) + payload.size());
	header.id = packetId;

	std::vector<char> sendBuffer(header.size);
	memcpy(sendBuffer.data(), &header, sizeof(PacketHeader));
	memcpy(sendBuffer.data() + sizeof(PacketHeader), payload.data(), payload.size());

	return Send(sendBuffer.data(), static_cast<int>(sendBuffer.size()));
}

std::string ChatClientSession::MakePayload(const char* buffer, int len)
{
	if (buffer == nullptr || len <= 0)
		return "";

	return std::string(buffer, buffer + len);
}

void ChatClientSession::PrintPacket(uint16 packetId, const std::string& payload)
{
	switch (packetId)
	{
	case S_LOGIN_RES:
		std::cout << "[S_LOGIN_RES] " << payload << std::endl;
		break;
	case S_ROOM_LIST_RES:
		std::cout << "[S_ROOM_LIST_RES] " << payload << std::endl;
		break;
	case S_ROOM_ENTER_RES:
		std::cout << "[S_ROOM_ENTER_RES] " << payload << std::endl;
		break;
	case S_ROOM_USER_JOIN:
		std::cout << "[S_ROOM_USER_JOIN] " << payload << std::endl;
		break;
	case S_ROOM_LEAVE_RES:
		std::cout << "[S_ROOM_LEAVE_RES] " << payload << std::endl;
		break;
	case S_ROOM_USER_LEAVE:
		std::cout << "[S_ROOM_USER_LEAVE] " << payload << std::endl;
		break;
	case S_CHAT:
		std::cout << "[S_CHAT] " << payload << std::endl;
		break;
	case S_ERROR:
		std::cout << "[S_ERROR] " << payload << std::endl;
		break;
	default:
		std::cout << "[UNKNOWN:" << packetId << "] " << payload << std::endl;
		break;
	}
}
