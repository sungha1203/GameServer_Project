#pragma once
#include <mutex>
#include "IocpCore.h"
#include "Listener.h"
#include "Session.h"

extern vector<std::shared_ptr<Session>> sessions;
extern std::atomic<int> g_connectedCnt;
extern mutex g_sessionsLock;

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
	std::unique_ptr<IocpCore>	iocpCore;
	std::unique_ptr <Listener>	listener;
	std::vector<thread>			workers;

	bool running = false;
};

