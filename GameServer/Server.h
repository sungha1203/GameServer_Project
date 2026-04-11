#pragma once
#include <mutex>
#include "IocpCore.h"
#include "Listener.h"
#include "Session.h"
#include "GameSession.h"
#include "SessionManager.h"
#include "GameSessionFactory.h"
#include "ConfigLoader.h"

class Server
{
public:
	Server();
	~Server();

	bool							Init();
	void							Start();
	void							End();
	void							ShutDown(const char* msg);

private:
	ConfigServer					config;

	std::unique_ptr<IocpCore>		iocpCore;
	std::unique_ptr<Listener>		listener;
	std::unique_ptr<SessionManager>	sessionManager;
	std::vector<thread>				workers;

	std::atomic<bool> running = false;
};