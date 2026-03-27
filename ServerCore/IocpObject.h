#pragma once

class IocpEvent;

// IOCP 완료 이벤트를 객체 단위로 처리하기 위한 클래스 (다형성을 위함.)
class IocpObject
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int numOfBytes = 0) abstract;
};