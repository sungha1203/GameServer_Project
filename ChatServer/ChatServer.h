#pragma once

#include <mutex>
#include "IocpCore.h"
#include "Listener.h"
#include "Session.h"
#include "SessionManager.h"
#include "ChatSessionFactory.h"
#include "ConfigLoader.h"

class ChatServer
{
public:
	ChatServer();
	~ChatServer();

	bool Init();
	void Start();
	void End();

private:
	ConfigServer config;

	std::unique_ptr<IocpCore> iocpCore;
	std::unique_ptr<Listener> listener;
	std::unique_ptr<SessionManager> sessionManager;
	std::vector<std::thread> workers;

	std::atomic<bool> running = false;
	std::atomic<bool> accepting = false;
};
