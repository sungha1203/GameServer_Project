#include "pch.h"
#include "ChatServer.h"
#include "ChatGlobal.h"
#include "ChatRoomManager.h"
#include "IocpCore.h"
#include "Listener.h"
#include "PacketProcessor.h"
#include "UserManager.h"
#include <atomic>
#include <mutex>

ChatServer::ChatServer()
{
}

ChatServer::~ChatServer()
{
}

bool ChatServer::Init()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;

	if (ConfigLoader<ConfigServer>::Load("ChatServerConfig.ini", config) == false)
	{
		PLOGE << "Failed to load chat server config";
		return false;
	}

	PLOGI << "ChatServer IP : " << config.ip;
	PLOGI << "ChatServer PORT : " << config.port;

	iocpCore = make_unique<IocpCore>();
	sessionManager = make_unique<SessionManager>(make_unique<ChatSessionFactory>(1000));
	GUserManager = std::make_unique<UserManager>();
	GRoomManager = std::make_unique<ChatRoomManager>();
	GRoomManager->Init(CHAT_ROOM_COUNT);
	PLOGI << "Chat rooms initialized. count=" << CHAT_ROOM_COUNT;

	listener = make_unique<Listener>(iocpCore.get(), sessionManager.get());
	listener->Init(config.ip, config.port);

	packetProcessor = make_unique<PacketProcessor>();
	packetProcessor->Start(4);

	return true;
}

void ChatServer::Start()
{
	running = true;
	accepting = true;

	const int num_core = thread::hardware_concurrency();
	for (int i = 0; i < num_core; ++i)
	{
		workers.emplace_back([this]()
			{
				while (running)
				{
					iocpCore->Dispatch();
				}
			});
	}
	PLOGI << "Chat server started";
}

void ChatServer::End()
{
	if (accepting.exchange(false) == false) return;

	if (listener)
		listener->Close();

	if (sessionManager)
	{
		auto sessions = sessionManager->GetActiveSessionsCopy();
		for (auto& session : sessions)
		{
			if (session)
				session->Disconnect();
		}
		sessions.clear();
	}

	while (1)
	{
		if (sessionManager->GetActiveSessionCnt() == 0)
			break;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	if (packetProcessor)
	{
		packetProcessor->Stop();
		packetProcessor.reset();
	}

	running = false;

	for (std::thread& worker : workers)
	{
		if (worker.joinable())
			worker.join();
	}
	workers.clear();

	if (sessionManager)
		PLOGI << "Active session count before shutdown : " << sessionManager->GetActiveSessionCnt();

	sessionManager.reset();
	GRoomManager.reset();
	GUserManager.reset();
	listener.reset();
	iocpCore.reset();

	WSACleanup();
}
