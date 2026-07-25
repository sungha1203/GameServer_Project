#include "pch.h"
#include <mutex>
#include "Listener.h"
#include "IocpObject.h"
#include "IocpCore.h"
#include "IocpEvent.h"
#include "SessionManager.h"
#include "Session.h"

Listener::Listener(IocpCore* core, SessionManager* sessionManager)
	: iocpCore(core), sessionManager(sessionManager)
{
}

Listener::~Listener()
{
}

void Listener::Init(const std::string& ip, int port)
{
	ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (ListenSocket == INVALID_SOCKET) return;

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	::inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

	if (::bind(ListenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) return;
	if (::listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) return;

	GUID guidAcceptEx = WSAID_ACCEPTEX;
	DWORD bytes = 0;
	if (WSAIoctl(ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx), &AcceptEx, sizeof(AcceptEx), &bytes, NULL, NULL) == SOCKET_ERROR) return;

	iocpCore->RegisterHandle(this);
	RegisterAccept();
}

void Listener::Close()
{
	if (ListenSocket == INVALID_SOCKET) return;

	closesocket(ListenSocket);
	ListenSocket = INVALID_SOCKET;
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(ListenSocket);
}

void Listener::Dispatch(IocpEvent* iocpEvent, int numOfBytes)
{
	if (iocpEvent->type == EventType::Accept)
		ProcessAccept(static_cast<AcceptEvent*>(iocpEvent));
}

void Listener::RegisterAccept()
{
	auto session = sessionManager->AcquireSession();
	if (session == nullptr) return;

	session->CreateSocket();

	AcceptEvent* AE = new AcceptEvent();
	AE->session = session;

	DWORD recvBytes = 0;
	bool ret = AcceptEx(ListenSocket, session->GetSocket(), AE->buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &recvBytes, &AE->overlapped);
	if (ret == FALSE)
	{
		const int err = ::WSAGetLastError();
		if (err != WSA_IO_PENDING)
		{
			if (session->GetSocket() != INVALID_SOCKET)
				closesocket(session->GetSocket());

			session->Reset();
			delete AE;
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* ae)
{
	SOCKET clientSocket = ae->session->GetSocket();

	if (SOCKET_ERROR == setsockopt(clientSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char*>(&ListenSocket), sizeof(ListenSocket)))
	{
		closesocket(clientSocket);
		delete ae;
		return;
	}

	iocpCore->RegisterHandle(ae->session.get());
	sessionManager->ActivateSession(ae->session);

	PLOGI << "Chat client connected : Session ID = " << ae->session->GetSessionId();
	ae->session->OnConnected();

	ae->session->RegisterRecv();
	RegisterAccept();

	delete ae;
}
