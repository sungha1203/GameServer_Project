#include "pch.h"
#include "Server.h"
#include "IocpCore.h"
#include "Listener.h"
#include "Session.h"
#include <atomic>
#include <mutex>

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

	if(ConfigLoader::Load("ServerConfig.ini", config) == false)
	{
		cout << "서버 설정 파일 로드 실패" << endl;
		PLOGI << "서버 설정 파일 로드 실패";
		return false;
	}

	cout << "IP : " << config.ip << endl;
	PLOGI << "IP : " << config.ip;
	cout << "PORT : " << config.port << endl << endl;
	PLOGI << "PORT : " << config.port;

	iocpCore = make_unique<IocpCore>();
	sessionManager = make_unique<SessionManager>();
	listener = make_unique<Listener>(iocpCore.get(), sessionManager.get());

	listener->Init(config.ip, config.port);
	return true;
}

void Server::Start()
{
	running = true;

	const int num_core = thread::hardware_concurrency();
	for(int i = 0; i < num_core; ++i)
	{
		workers.emplace_back([this]()
		{
			while (running)
			{
				iocpCore->Dispatch();
			}
			});
	}

}

void Server::End()
{
	running = false;

	for(std::thread& worker : workers)
	{
		if(worker.joinable())
		{
			worker.join();
		}
	}

	cout << "서버 종료 완료" << endl;
}

void Server::ShutDown(const char* msg)
{
	for (auto& session : sessionManager->GetSessions())
	{
		if (session == nullptr)
			continue;

		SOCKET s = session->GetSocket();
		if (s == INVALID_SOCKET)
			continue;

		int ret = send(s, msg, static_cast<int>(strlen(msg)), 0);
	}
}