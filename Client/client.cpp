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

	if (ConfigLoader<ConfigClient>::Load("ClientConfig.ini", config) == false)
	{
		PLOGE << "서버 설정 파일 로드 실패";
		return false;
	}

	iocpCore = make_unique<IocpCore>();
	connector = make_unique<Connector>(iocpCore.get());

	sessionManager = make_unique<SessionManager>(std::make_unique<ClientSessionFactory>(config.clientCnt));

	return true;
}

bool Client::ConnectClients()
{
	if (sessionManager == nullptr || connector == nullptr) return false;

	clientSessions.reserve(config.clientCnt);

	int successCnt = 0;

	for (int i = 0; i < config.clientCnt; ++i)
	{
		auto session = sessionManager->AcquireSession();

		if (session == nullptr)
			continue;

		session->SetSessionId(i + 1);

		if (connector->Connect(session, config.ip.c_str(), config.port))
		{
			sessionManager->ActivateSession(session);
			clientSessions.push_back(session);
			++successCnt;
		}
	}

	PLOGI << successCnt << "개의 클라이언트가 서버에 연결되었습니다.";

	return successCnt > 0;
}

void Client::Start()
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

	sendThread = std::thread([this]()
		{
			while (running)
			{
				BroadcastChat();
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		});

	// PLOGI << "클라이언트 시작!";
}

void Client::BroadcastChat()
{
	for (auto& session : clientSessions)
	{
		auto clientSession = std::static_pointer_cast<ClientSession>(session);
		if (clientSession)
			clientSession->SendChat("ㅎㅇ");
	}
}

void Client::End()
{
	running = false;

	if(sendThread.joinable())
		sendThread.join();

	for (auto& session : clientSessions)
	{
		if (session)
			session->Disconnect();
	}

	for (auto& worker : workers)
	{
		if (worker.joinable())
			worker.join();
	}

	workers.clear();
	clientSessions.clear();

	// PLOGI << "클라이언트 종료!";
}