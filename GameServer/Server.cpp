#include "pch.h"
#include "Server.h"
#include "IocpCore.h"
#include "Listener.h"
#include "PacketProcessor.h"
#include <atomic>
#include <mutex>

std::atomic<int> recvCnt = 0;

Server::Server()
{
}

Server::~Server()
{
}

bool Server::Init()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;

	if (ConfigLoader<ConfigServer>::Load("ServerConfig.ini", config) == false)
	{
		PLOGE << "서버 설정 파일 로드 실패";
		return false;
	}

	PLOGI << "IP : " << config.ip;
	PLOGI << "PORT : " << config.port;

	// 서버 종료시 삭제되어야 해서 unique_ptr로 관리, 포인터만 들고 있고 소유X
	iocpCore = make_unique<IocpCore>();
	sessionManager = make_unique<SessionManager>(make_unique<GameSessionFactory>(1000));

	listener = make_unique<Listener>(iocpCore.get(), sessionManager.get());
	listener->Init(config.ip, config.port);

	packetProcessor = make_unique<PacketProcessor>();					 // new
	packetProcessor->Start(8);											 // new

	return true;
}

void Server::Start()
{
	running = true;

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
	PLOGI << "서버 시작!";
}

void Server::End()
{
	if (running.exchange(false) == false) return;

	if (listener)
	{
		listener->Close();
	}

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

	//PLOGE << "[End Check] active=" << sessionManager->GetActiveSessionCnt() << ", packetQ=" << packetProcessor->packetQueue.size();

	std::this_thread::sleep_for(std::chrono::milliseconds(300));

	for (std::thread& worker : workers)
	{
		if (worker.joinable())
			worker.join();
	}
	workers.clear();

	if (packetProcessor)
	{
		packetProcessor->Stop();
		packetProcessor.reset();
	}

	sessionManager.reset();
	listener.reset();
	iocpCore.reset();


	WSACleanup();
}