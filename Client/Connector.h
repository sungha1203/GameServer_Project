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

	bool Connect(std::shared_ptr<Session> session, const std::string& ip, int port);

private:
	IocpCore* iocpCore = nullptr;
};

