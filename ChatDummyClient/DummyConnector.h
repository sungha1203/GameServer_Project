#pragma once

#include "IocpCore.h"
#include "Session.h"

class DummyConnector
{
public:
	DummyConnector(IocpCore* iocpCore);

	bool Connect(std::shared_ptr<Session> session, const std::string& ip, int port);

private:
	IocpCore* iocpCore = nullptr;
};
