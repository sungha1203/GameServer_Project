#pragma once
#include <mutex>
#include "IocpCore.h"
#include "Listener.h"
#include "Session.h"
#include "SessionManager.h"

class Server
{
public:
	Server();
	~Server();

	void Init();
	void Start();
	void End();
	void ShutDown(const char* msg);

private:
	std::unique_ptr<IocpCore>		iocpCore;
	std::unique_ptr<Listener>		listener;
	std::unique_ptr<SessionManager>	sessionManager;
	std::vector<thread>				workers;

	bool running = false;
};

