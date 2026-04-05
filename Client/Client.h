#pragma once
#include "pch.h"
#include "IocpCore.h"
#include "Connector.h"
#include "ClientSession.h"

class Client
{
public:
	Client();
	~Client();

	bool								Init();
	bool								Connect();
	void								Start();
	void								End();

private:
	std::unique_ptr<IocpCore>			iocpCore;
	std::unique_ptr<Connector>			connector;
	std::unique_ptr<ClientSession>		clientSession;
	std::vector<thread>					workers;
};