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
	PLOGI << "서버 시작!";
}

void Server::End()
{
	if (accepting.exchange(false) == false) return;

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

	// ICOP 스레드가 세션에서 패킷 처리 중일 수 있으니 세션이 모두 종료될 때까지 대기
	while (1)
	{
		if(sessionManager->GetActiveSessionCnt() == 0)
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
	{
		PLOGE << "세션 매니저 종료 전 active 세션 수 : " << sessionManager->GetActiveSessionCnt();
	}

	sessionManager.reset();
	listener.reset();
	iocpCore.reset();

	WSACleanup();
}