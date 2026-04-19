#include "Connector.h"

Connector::Connector(IocpCore* iocpCore)
	:iocpCore(iocpCore)
{

}

Connector::~Connector()
{
}

bool Connector::Connect(std::shared_ptr<Session> session, const std::string& ip, int port)
{
	session->CreateSocket();

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);

	if(inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) != 1)	return false;

	if(connect(session->GetSocket(), reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		PLOGE<< "연결 실패 : " << err;
		return false;
	}

	if(!iocpCore->RegisterHandle(session.get()))
	{
		PLOGE << "IOCP 등록 실패";
		return false;
	}

	session->RegisterRecv();

	return true;
}