#pragma once
#include "IocpObject.h"
#include "IocpEvent.h"

class Session : public IocpObject
{
public:
	Session();
	~Session();

public:
	void			CreateSocket();

public:
	SOCKET			GetSocket() { return socket; }

	void			RegisterRecv();
	void			ProcessRecv(int numOfBytes);

	void			Disconnect();

public:
	virtual HANDLE	GetHandle() override;
	virtual void	Dispatch(class IocpEvent* iocpEvent, int numOfBytes = 0) override;

private:
	SOCKET			socket = INVALID_SOCKET;
	RecvEvent		recvEvent;
};

