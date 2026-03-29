#pragma once
#include "IocpObject.h"
#include "IocpEvent.h"

using uint16 = unsigned short;

class SessionManager;

struct PacketHeader
{
	uint16 size;		// 전체 패킷 크기 (헤더 + 데이터)
	uint16 id;			// 패킷 종류
};

enum : uint16
{
	PKT_CHAT = 1,
};

class Session : public IocpObject , public std::enable_shared_from_this<Session>
{
public:
	Session();
	virtual ~Session();

public:
	void					SetSessionId(int id);
	int						GetSessionId() { return sessionId; }
	void					SetSessionManager(SessionManager* manager) { sessionManager = manager; }

public:
	void					CreateSocket();
	SOCKET					GetSocket() { return socket; }

	void					RegisterRecv();
	void					ProcessRecv(int numOfBytes);
	void					ProcessPacket();

	void					Disconnect();
	void					Reset();

public:
	virtual HANDLE			GetHandle() override;
	virtual void			Dispatch(class IocpEvent* iocpEvent, int numOfBytes = 0) override;

private:
	SOCKET					socket = INVALID_SOCKET;

	int						sessionId = 0;
	SessionManager*			sessionManager = nullptr;
	std::atomic<bool>		isConnected{ false };
	
	RecvEvent				recvEvent;

	char					packetBuffer[4096]{};
	int						packetBufferSize = 0;	
};