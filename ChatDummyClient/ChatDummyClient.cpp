#include "pch.h"
#include "ChatDummyClient.h"

#include "DummyConfigLoader.h"

#include <iostream>

ChatDummyClient::ChatDummyClient()
{
}

ChatDummyClient::~ChatDummyClient()
{
	End();
}

bool ChatDummyClient::Init()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	if (!DummyConfigLoader::Load("ChatDummyConfig.ini", config))
	{
		PLOGE << "Failed to load ChatDummyConfig.ini";
		return false;
	}

	if (config.clientCount <= 0 || config.connectThreadCount <= 0 || config.roomCount <= 0 || config.sendPerSecond < 0 || config.testSeconds <= 0)
	{
		PLOGE << "Invalid dummy config";
		return false;
	}

	iocpCore = std::make_unique<IocpCore>();
	connector = std::make_unique<DummyConnector>(iocpCore.get());
	packetProcessor = std::make_unique<PacketProcessor>();
	packetProcessor->Start(config.packetProcessorThreadCount);
	sessionManager = std::make_unique<SessionManager>(std::make_unique<DummySessionFactory>(config.clientCount));

	sessions.resize(config.clientCount);

	if (!metrics.OpenCsv("DummyResult.csv"))
	{
		PLOGE << "Failed to open DummyResult.csv";
		return false;
	}

	return true;
}

bool ChatDummyClient::Start()
{
	running = true;
	accepting = true;

	StartWorkers();

	const int threadCount = config.connectThreadCount;
	const int sessionsPerThread = (config.clientCount + threadCount - 1) / threadCount;

	for (int i = 0; i < threadCount; ++i)
	{
		const int begin = i * sessionsPerThread;
		const int end = min(config.clientCount, begin + sessionsPerThread);
		if (begin >= end)
			continue;

		connectThreads.emplace_back(&ChatDummyClient::ConnectRange, this, begin, end);
	}

	for (std::thread& th : connectThreads)
	{
		if (th.joinable())
			th.join();
	}

	sendThread = std::thread(&ChatDummyClient::SendLoop, this);
	metricsThread = std::thread(&ChatDummyClient::MetricsLoop, this);

	return true;
}

void ChatDummyClient::Run()
{
	std::this_thread::sleep_for(std::chrono::seconds(config.testSeconds));
}

void ChatDummyClient::End()
{
	if (accepting.exchange(false) == false)
		return;

	running = false;

	if (sendThread.joinable())
		sendThread.join();

	if (metricsThread.joinable())
		metricsThread.join();

	for (std::shared_ptr<DummySession>& session : sessions)
	{
		if (session)
			session->Disconnect();
	}

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

	sessions.clear();
	sessionManager.reset();
	connector.reset();
	iocpCore.reset();

	WSACleanup();
}

void ChatDummyClient::ConnectRange(int begin, int end)
{
	for (int i = begin; i < end && running; ++i)
	{
		std::shared_ptr<Session> baseSession = sessionManager->AcquireSession();
		baseSession->SetSessionManager(sessionManager.get());

		std::shared_ptr<DummySession> dummySession = std::static_pointer_cast<DummySession>(baseSession);
		const int roomId = (i % config.roomCount) + 1;
		dummySession->InitDummy(i + 1, roomId, &metrics);

		if (!connector->Connect(dummySession, config.ip, config.port))
		{
			metrics.OnError();
			continue;
		}

		sessionManager->ActivateSession(dummySession);
		dummySession->OnConnected();
		sessions[i] = dummySession;
	}
}

void ChatDummyClient::StartWorkers()
{
	const unsigned int hardwareCount = std::thread::hardware_concurrency();
	const int workerCount = static_cast<int>(hardwareCount == 0 ? 1 : hardwareCount);

	for (int i = 0; i < workerCount; ++i)
	{
		workers.emplace_back([this]()
			{
				while (running)
					iocpCore->Dispatch();
			});
	}
}

void ChatDummyClient::SendLoop()
{
	if (config.sendPerSecond == 0)
		return;

	const auto interval = std::chrono::milliseconds(1000);

	while (running)
	{
		const auto begin = std::chrono::steady_clock::now();

		for (int i = 0; i < config.sendPerSecond && running; ++i)
		{
			for (std::shared_ptr<DummySession>& session : sessions)
			{
				if (session)
					session->SendChatTick();
			}
		}

		const auto elapsed = std::chrono::steady_clock::now() - begin;
		if (elapsed < interval)
			std::this_thread::sleep_for(interval - elapsed);
	}
}

void ChatDummyClient::MetricsLoop()
{
	for (int elapsed = 1; running && elapsed <= config.testSeconds; ++elapsed)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		metrics.WriteCsvRow(elapsed);
	}
}
