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

	sessionManager = make_unique<SessionManager>(std::make_unique<ClientSessionFactory>(config.connectThreadCnt * config.sessionCntPerThread));

	return true;
}

bool Client::ConnectClients()
{
	if (sessionManager == nullptr || connector == nullptr) return false;

	clientSessions.reserve(config.connectThreadCnt * config.sessionCntPerThread); // 10000
	connectThreads.reserve(config.connectThreadCnt);							  // 10

	for (int i = 0; i < config.connectThreadCnt; ++i)
	{
		connectThreads.emplace_back(&Client::ConnectThread, this, i, config.sessionCntPerThread);
	}

	for (auto& th : connectThreads)
	{
		if (th.joinable())
			th.join();
	}

	return true;
}

void Client::ConnectThread(int threadidx, int sessionCntPerThread)
{
	std::vector<std::shared_ptr<Session>> localSessions;		// 임시 로컬 세션 벡터, 나중에 합칠거임.
	localSessions.reserve(sessionCntPerThread);

	for (int i = 0; i < sessionCntPerThread; ++i)
	{
		std::shared_ptr<Session> session = sessionManager->AcquireSession();
		session->SetSessionManager(sessionManager.get());

		if (connector->Connect(session, config.ip, config.port) == false)
		{
			PLOGE << "세션 연결 실패";
			continue;
		}

		sessionManager->ActivateSession(session);
		localSessions.push_back(session);
	}

	// 메인 세션 벡터에 로컬 세션 벡터를 합침.
	{
		std::lock_guard<std::mutex> lock(sessionLock);
		for (auto& session : localSessions)
		{
			clientSessions.push_back(session);
		}
	}
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
		// 클라세션에 있는 기능을 사용하려고 다운 캐스팅을 함.
		auto clientSession = std::static_pointer_cast<ClientSession>(session);

		if (clientSession->IsConnected() == false)
		{
			if(ReconnectSession(clientSession))
				PLOGI << "세션 재연결 성공 : " << clientSession->GetSessionId();
			 else
				PLOGE << "세션 재연결 실패 : " << clientSession->GetSessionId();

			continue;
		}

		if (clientSession)
		{
			for (int i = 0; i < 10; ++i) {
				clientSession->SendChat("ㅎㅇ");
				++sendCnt;
				if(sendCnt % 1000 == 0)
					PLOGE << "보낸 메시지 수 : " << sendCnt;
			}
		}
		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

bool Client::ReconnectSession(std::shared_ptr<ClientSession> session)
{
	if (session == nullptr) return false;

	session->Reset(); // 재사용 전 초기화

	if(!connector->Connect(session, config.ip, config.port))
		return false;
	
	// 재연결 성공 시 세션 매니저에 다시 활성화
	sessionManager->ActivateSession(session);

	return true;
}

void Client::End()
{
	running = false;

	if (sendThread.joinable())
		sendThread.join();

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
	connectThreads.clear();
	clientSessions.clear();

	// PLOGI << "클라이언트 종료!";
	WSACleanup();
}