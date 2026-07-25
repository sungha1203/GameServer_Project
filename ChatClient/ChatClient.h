#pragma once

#include "ChatClientSession.h"
#include "ChatClientSessionFactory.h"
#include "ChatConnector.h"
#include "ConfigLoader.h"
#include "IocpCore.h"
#include "PacketProcessor.h"
#include "SessionManager.h"

class ChatClient
{
public:
	ChatClient();
	~ChatClient();

	bool Init();
	bool Connect();
	void Start();
	void RunConsole();
	void End();

private:
	void PrintCommands();
	void HandleCommand(const std::string& line);
	std::shared_ptr<ChatClientSession> GetSession();

private:
	ConfigServer config;

	std::unique_ptr<IocpCore> iocpCore;
	std::unique_ptr<ChatConnector> connector;
	std::unique_ptr<PacketProcessor> packetProcessor;
	std::unique_ptr<SessionManager> sessionManager;
	std::shared_ptr<ChatClientSession> session;

	std::vector<std::thread> workers;
	std::atomic<bool> running = false;
};
