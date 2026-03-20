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

void Server::Init()
{
	iocpCore = make_unique<IocpCore>();
	sessionManager = make_unique<SessionManager>();
	listener = make_unique<Listener>(iocpCore.get(), sessionManager.get());

	listener->Init();
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