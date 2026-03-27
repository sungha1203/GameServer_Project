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
	Session(SessionManager* sessionManager);
	~Session();

public:
	void			CreateSocket();

public:
	SOCKET			GetSocket() { return socket; }

	void			RegisterRecv();
	void			ProcessRecv(int numOfBytes);
	void			ProcessPacket();

	void			Disconnect();

public:
	virtual HANDLE	GetHandle() override;
	virtual void	Dispatch(class IocpEvent* iocpEvent, int numOfBytes = 0) override;

private:
	SOCKET			socket = INVALID_SOCKET;
	SessionManager* sessionManager = nullptr;
	
	RecvEvent		recvEvent;

	char			packetBuffer[4096]{};
	int				packetBufferSize = 0;	
};