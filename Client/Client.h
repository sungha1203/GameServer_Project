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
	void											ConnectThread(int threadidx, int sessionCntPerThread);
	void											Start();
	void											BroadcastChat();
	bool											ReconnectSession(std::shared_ptr<ClientSession> session);
	void											End();


private:
	ConfigClient									config;

	std::unique_ptr<IocpCore>						iocpCore;
	std::unique_ptr<Connector>						connector;
	std::unique_ptr<SessionManager>					sessionManager;

	std::vector<std::shared_ptr<Session>>			clientSessions;		// 1000

private:
	std::vector<thread>								workers;
	std::vector<std::thread>						connectThreads;
	std::thread										sendThread;
	std::atomic<bool>								running = false;
	std::mutex										sessionLock;

private:
	std::atomic<int>								sendCnt = 0;		// 보낸 메시지 수
};