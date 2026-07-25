#pragma once

#include "DummyConfig.h"
#include "DummyConnector.h"
#include "DummyMetrics.h"
#include "DummySession.h"
#include "DummySessionFactory.h"
#include "IocpCore.h"
#include "PacketProcessor.h"
#include "SessionManager.h"

class ChatDummyClient
{
public:
	ChatDummyClient();
	~ChatDummyClient();

	bool Init();
	bool Start();
	void Run();
	void End();

private:
	void ConnectRange(int begin, int end);
	void StartWorkers();
	void SendLoop();
	void MetricsLoop();

private:
	DummyConfig config;
	DummyMetrics metrics;

	std::unique_ptr<IocpCore> iocpCore;
	std::unique_ptr<DummyConnector> connector;
	std::unique_ptr<PacketProcessor> packetProcessor;
	std::unique_ptr<SessionManager> sessionManager;

	std::vector<std::shared_ptr<DummySession>> sessions;
	std::vector<std::thread> workers;
	std::vector<std::thread> connectThreads;
	std::thread sendThread;
	std::thread metricsThread;

	std::atomic<bool> running = false;
	std::atomic<bool> accepting = false;
};
