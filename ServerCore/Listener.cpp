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
	if (INVALID_SOCKET != ListenSocket)
	{
		closesocket(ListenSocket);
	}
	ListenSocket = INVALID_SOCKET;
}

void Listener::Init(const std::string& ip, int port)
{
	ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (ListenSocket == INVALID_SOCKET) return;

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	::inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);
	//serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(ListenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) return;
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) return;

	// AcceptEx 함수 포인터 로딩
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	DWORD bytes = 0;

	// 초기화 함수. SIO_GET_EXTENSION_FUNCTION_POINTER가 필수적으로 들어가야함. 
	// AcceptEx 함수 주소를 가져와서 멤버 변수 AcceptEx에 저장. 이후 AcceptEx 함수를 사용할 때 이 포인터를 통해 호출.
	if (WSAIoctl(ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx), &AcceptEx, sizeof(AcceptEx), &bytes, NULL, NULL) == SOCKET_ERROR) { return; }

	// listen socket IOCP에 등록
	iocpCore->RegisterHandle(this);

	// 10개 accept등록
	for (int i = 0; i < 10; ++i)
	{
		RegisterAccept();
	}
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(ListenSocket);
}

void Listener::Dispatch(IocpEvent* iocpEvent, int numOfBytes)
{
	if (iocpEvent->type == EventType::Accept)
	{
		ProcessAccept(static_cast<AcceptEvent*>(iocpEvent));
	}
}

void Listener::RegisterAccept()
{
	auto session = sessionManager->AcquireSession();
	//std::shared_ptr<Session> session = std::make_shared<Session>(sessionManager);
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
			delete AE;
		}
	}
}

// client 접속 완료 처리
void Listener::ProcessAccept(AcceptEvent* ae)
{
	SOCKET clientSocket = ae->session->GetSocket();

	if (SOCKET_ERROR == setsockopt(clientSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&ListenSocket, sizeof(ListenSocket)))
	{
		closesocket(clientSocket);
		delete ae;
		return;
	}

	// 접속된 클라이언트 소켓을 IOCP에 등록하여 이후 통신에서 IOCP 이벤트가 발생하도록 함
	iocpCore->RegisterHandle(ae->session.get());

	sessionManager->ActivateSession(ae->session);

	//PLOGI << "Client 연결 : Session ID = " << ae->session->GetSessionId();

	// 클라이언트로부터 데이터를 받기 위한 비동기 recv 요청 등록
	ae->session->RegisterRecv();

	// 다음 클라이언트 접속을 기다리는 Accept 요청 등록
	RegisterAccept();

	delete ae;
}