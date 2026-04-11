#pragma once
#include "pch.h"
#include "IocpCore.h"
#include "Connector.h"
#include "ClientSession.h"
#include "ConfigLoader.h"
#include "SessionManager.h"

class Client
{
public:
	Client();
	~Client();

	bool								Init();
	void								Start();
	void								End();

public:
	std::shared_ptr<Session>			GetSession() const { return clientSession; }

private:
	Config								config;

	std::unique_ptr<IocpCore>			iocpCore;
	std::unique_ptr<Connector>			connector;
	std::unique_ptr<SessionManager>		sessionManager;

	std::shared_ptr<Session>			clientSession;   // 일단 클라 하나만 생각해보자.
	std::vector<thread>					workers;
	std::atomic<bool>					running = false;
};