#pragma once

#include "IocpCore.h"
#include "Session.h"

class ChatConnector
{
public:
	ChatConnector(IocpCore* iocpCore);
	~ChatConnector();

	bool Connect(std::shared_ptr<Session> session, const std::string& ip, int port);

private:
	IocpCore* iocpCore = nullptr;
};
