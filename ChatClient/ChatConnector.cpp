#include "pch.h"
#include "ChatConnector.h"

ChatConnector::ChatConnector(IocpCore* iocpCore)
	: iocpCore(iocpCore)
{
}

ChatConnector::~ChatConnector()
{
}

bool ChatConnector::Connect(std::shared_ptr<Session> session, const std::string& ip, int port)
{
	if (session == nullptr || iocpCore == nullptr)
		return false;

	session->CreateSocket();

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);

	if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) != 1)
		return false;

	if (connect(session->GetSocket(), reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
	{
		PLOGE << "Connect failed. error=" << WSAGetLastError();
		return false;
	}

	if (!iocpCore->RegisterHandle(session.get()))
	{
		PLOGE << "IOCP register failed";
		return false;
	}

	session->RegisterRecv();
	return true;
}
