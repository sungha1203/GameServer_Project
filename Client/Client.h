#pragma once
#include "pch.h"
#include "IocpCore.h"
#include "Connector.h"
#include "ClientSession.h"
#include "ConfigLoader.h"
#include "SessionManager.h"
#include "ClientSessionFactory.h"

class Client
{
public:
	Client();
	~Client();

	bool											Init();
	bool											ConnectClients();
	void											Start();
	void											BroadcastChat();
	void											End();


private:
	ConfigClient									config;

	std::unique_ptr<IocpCore>						iocpCore;
	std::unique_ptr<Connector>						connector;
	std::unique_ptr<SessionManager>					sessionManager;

	std::vector<std::shared_ptr<Session>>			clientSessions;		// 1000
	std::vector<thread>								workers;
	std::thread										sendThread;
	std::atomic<bool>								running = false;
};