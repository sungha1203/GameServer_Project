#pragma once
#include "IocpObject.h"

class IocpObject;

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

public:
	HANDLE		GetIocpHandle() const { return h_iocp; }

	bool		RegisterHandle(IocpObject* iocpObject);
	bool		Dispatch();

private:
	HANDLE		h_iocp;
};

