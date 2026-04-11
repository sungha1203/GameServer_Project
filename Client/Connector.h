#pragma once
#include "pch.h"
#include "IocpCore.h"
#include "ClientSession.h"

class IocpCore;
class ClientSession;

class Connector
{
public:
	Connector(IocpCore* iocpCore);
	~Connector();

	bool Connect(std::shared_ptr<Session> session, const char* ip, int port);

private:
	IocpCore* iocpCore = nullptr;
};

