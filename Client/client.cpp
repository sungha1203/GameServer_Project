#include "Client.h"

Client::Client()
{
}

Client::~Client()
{
}

bool Client::Init()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;

	if (ConfigLoader::Load("ClientConfig.ini", config) == false)
	{
		PLOGE << "서버 설정 파일 로드 실패";
		return false;
	}

	iocpCore = make_unique<IocpCore>();
	connector = make_unique<Connector>(iocpCore.get());

	sessionManager = make_unique<SessionManager>(
		[]()->std::shared_ptr<Session>
		{
			return std::make_shared<ClientSession>();
		});
	clientSession = sessionManager->AcquireSession();
	clientSession->SetSessionId(1);

	connector->Connect(clientSession, config.ip.c_str(), config.port);

	return true;
}

void Client::Start()
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
	// PLOGI << "클라이언트 시작!";
}

void Client::End()
{
	running = false;

	if (clientSession)
	{
		clientSession->Disconnect();
	}

	for (auto& worker : workers)
	{
		if (worker.joinable())
			worker.join();
	}

	workers.clear();

	// PLOGI << "클라이언트 종료!";
}