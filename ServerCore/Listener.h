#pragma once
#include "IocpObject.h"

class IocpCore;
class AcceptEvent;
class Session;
class SessionManager;

class Listener : public IocpObject
{
public:
	Listener(IocpCore* core, SessionManager* sessionManager);
	~Listener();

	void				Init(const std::string& ip, int port);

public:
	virtual HANDLE		GetHandle() override;
	virtual void		Dispatch(class IocpEvent* iocpEvent, int numOfBytes = 0) override;

public:
	void				RegisterAccept();					// 비동기 Accept 요청 등록
	void				ProcessAccept(AcceptEvent* ae);	// 접속 완료된 클라이언트 처리

private:
	SOCKET				ListenSocket = INVALID_SOCKET;
	IocpCore*			iocpCore = nullptr;
	SessionManager*		sessionManager = nullptr;

	LPFN_ACCEPTEX		AcceptEx = nullptr;
};