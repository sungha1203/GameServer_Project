#include "pch.h"
#include <algorithm>
#include <mutex>
#include "Session.h"
#include "IocpObject.h"
#include "IocpEvent.h"

extern std::atomic<int> g_connectedCnt;
extern vector<std::shared_ptr<Session>> sessions;
extern mutex g_sessionsLock;

Session::Session()
{
}

Session::~Session()
{
}

void Session::CreateSocket()
{
	socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

void Session::RegisterRecv()
{
	// recvEvent 초기화
	ZeroMemory(&recvEvent.overlapped, sizeof(OVERLAPPED));

	WSABUF wsaBuf;
	wsaBuf.buf = recvEvent.buffer;
	wsaBuf.len = sizeof(recvEvent.buffer);

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	if (SOCKET_ERROR == WSARecv(socket, &wsaBuf, 1, nullptr, &flags, &recvEvent.overlapped, nullptr))
	{
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
		{
			cout << "WSARecv 실패 : " << err << endl;
		}
	}
}

void Session::ProcessRecv(int numOfBytes)
{
	if (numOfBytes == 0)
	{
		if (socket != INVALID_SOCKET)
		{
			Disconnect();
			return;
		}
	}

	// recv 재등록
	RegisterRecv();
}

void Session::Disconnect()
{
	if (socket == INVALID_SOCKET)
	{
		return;
	}
	closesocket(socket);
	socket = INVALID_SOCKET;
	
	std::lock_guard<std::mutex> lock(g_sessionsLock);
	sessions.erase(
		std::remove_if(
			sessions.begin(),
			sessions.end(),
			[this](const std::shared_ptr<Session>& s)
			{
				return s.get() == this;
			}),
		sessions.end()
	);

	int disconnected = --g_connectedCnt;
	if (disconnected % 100 == 0)
		cout << "Disconnected Cnt = " << disconnected << endl << endl;
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(GetSocket());
}

void Session::Dispatch(IocpEvent* iocpEvent, int numOfBytes)
{
	if (iocpEvent->type == EventType::Recv)
	{
		ProcessRecv(numOfBytes);
	}
}