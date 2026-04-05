#include "pch.h"
#include <algorithm>
#include <mutex>
#include "SessionManager.h"
#include "Session.h"
#include "IocpObject.h"
#include "IocpEvent.h"

Session::Session()
{
}

Session::~Session()
{
}

void Session::SetSessionId(int id)
{
	sessionId = id;
}

void Session::CreateSocket()
{
	socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//isConnected = true;
}

void Session::RegisterRecv()
{
	// recvEvent 초기화
	ZeroMemory(&recvEvent.overlapped, sizeof(recvEvent.overlapped));
	ZeroMemory(&recvEvent.buffer, sizeof(recvEvent.buffer));

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
			PLOGE << "WSARecv 실패 : " << err << endl;
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
}

void Session::Disconnect()
{
	//if (isConnected.exchange(false) == false) return;

	if (socket != INVALID_SOCKET)
	{
		shutdown(socket, SD_BOTH);
		closesocket(socket);
		socket = INVALID_SOCKET;
	}

	//PLOGW << "Session Disconnected : " << sessionId;

	if (sessionManager)
		sessionManager->ReleaseSession(shared_from_this());
}

void Session::Reset()
{
	socket = INVALID_SOCKET;
	sessionId = 0;
	//isConnected = false;

	ZeroMemory(&recvEvent.overlapped, sizeof(recvEvent.overlapped));
	ZeroMemory(recvEvent.buffer, sizeof(recvEvent.buffer));
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(GetSocket());
}

void Session::Dispatch(IocpEvent* iocpEvent, int numOfBytes)
{
	switch (iocpEvent->type)
	{
	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;
	default:
		break;
	}
}