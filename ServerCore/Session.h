#pragma once
#include <WinSock2.h>
#include <memory>
#include "IocpObject.h"
#include "IocpEvent.h"

using uint16 = unsigned short;

class SessionManager;

struct PacketHeader
{
	uint16 size;		// 전체 패킷 크기 (헤더 + 데이터)
	uint16 id;			// 패킷 종류
};

class Session : public IocpObject , public std::enable_shared_from_this<Session>
{
public:
	Session();
	virtual ~Session();

public:
	void							SetSessionId(int id);
	int								GetSessionId() { return sessionId; }
	void							SetSessionManager(SessionManager* manager) { sessionManager = manager; }

public:
	void							CreateSocket();
	SOCKET							GetSocket() { return socket; }

	void							RegisterRecv();
	virtual void					ProcessRecv(int numOfBytes);

	bool							Send(const char* buffer, int len);
	void							RegisterSend();
	void							ProcessSend(SendEvent* sendEvent, int numOfBytes);


	void							Disconnect();
	virtual void					Reset();

public:
	virtual HANDLE					GetHandle() override;
	virtual void					Dispatch(class IocpEvent* iocpEvent, int numOfBytes = 0) override;

protected:
	SOCKET							socket = INVALID_SOCKET;

	int								sessionId = 0;
	SessionManager*					sessionManager = nullptr;
	std::atomic<bool>				isConnected{ false };		// 소켓이 준비 됐음.
	
	RecvEvent						recvEvent;
	
	std::queue<std::vector<char>>	sendQueue;
	std::mutex						sendMutex;
	std::atomic<bool>				isSending{ false };
};