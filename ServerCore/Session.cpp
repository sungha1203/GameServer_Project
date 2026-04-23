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
	isConnected = true;
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

// 전송 요청
bool Session::Send(const char* buffer, int len)
{
	// 접속X 세션은 가셈
	if (isConnected == false)	return false;
	// 보낼 데이터가 없거나 길이가 0이하도 가셈
	if(buffer == nullptr || len <= 0)	return false;

	std::vector<char> sendData(buffer, buffer + len);

	{
		std::lock_guard<std::mutex> lock(sendMutex);
		sendQueue.push(std::move(sendData));
	}

	RegisterSend();
	return true;
}

// 중복 없이 비동기 send 등록
void Session::RegisterSend()
{
	// 이미 전송 중이면 WSASend 등록 안함
	if (isSending.exchange(true) == true) return;

	std::vector<char> sendData;

	{
		std::lock_guard<std::mutex> lock(sendMutex);

		if(sendQueue.empty())
		{
			isSending = false;
			return;
		}

		sendData = std::move(sendQueue.front());
		sendQueue.pop();
	}

	SendEvent* sendEvent = new SendEvent();
	sendEvent->sendbuffer = std::move(sendData);

	WSABUF wsaBuf;
	wsaBuf.buf = sendEvent->sendbuffer.data();
	wsaBuf.len = static_cast<ULONG>(sendEvent->sendbuffer.size());

	DWORD sendBytes = 0;

	int ret = WSASend(socket, &wsaBuf, 1, &sendBytes, 0, &sendEvent->overlapped, nullptr);

	if(ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
		{
			PLOGE << "WSASend 실패 : " << err << endl;
			delete sendEvent;
			isSending = false;
			Disconnect();
		}
	}
}

// send 완료 이벤트 처리
void Session::ProcessSend(SendEvent* sendEvent, int numOfBytes)
{
	delete sendEvent;

	{
		std::lock_guard<std::mutex> lock(sendMutex);

		if (sendQueue.empty())
		{
			isSending = false;
			return;
		}
	}

	// 아직 큐에 보낼 데이터가 남아있음 -> 다음 send 등록
	isSending = false;
	RegisterSend();
}

void Session::Disconnect()
{
	if (isConnected.exchange(false) == false) return;

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
	isConnected = false;
	sessionManager = nullptr;

	ZeroMemory(&recvEvent.overlapped, sizeof(recvEvent.overlapped));
	ZeroMemory(recvEvent.buffer, sizeof(recvEvent.buffer));

	{
		std::lock_guard<std::mutex> lock(sendMutex);
		while (!sendQueue.empty())
			sendQueue.pop();
	}
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
	case EventType::Send:
		ProcessSend(static_cast<SendEvent*>(iocpEvent), numOfBytes);
		break;
	default:
		break;
	}
}