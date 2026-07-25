#pragma once

#include <atomic>
#include <fstream>
#include <string>

struct DummyMetricsSnapshot
{
	long long sent = 0;
	long long received = 0;
	long long latencyCount = 0;
	long long totalLatencyMs = 0;
	long long maxLatencyMs = 0;
};

class DummyMetrics
{
public:
	void OnConnected();
	void OnDisconnected();
	void OnLoggedIn();
	void OnEnterRoom();
	void OnSent();
	void OnReceived();
	void OnError();
	void OnLatency(long long latencyMs);

	int GetConnected() const { return connected.load(); }
	int GetLoggedIn() const { return loggedIn.load(); }
	int GetInRoom() const { return inRoom.load(); }

	bool OpenCsv(const std::string& filePath);
	void WriteCsvRow(int elapsedSeconds);

private:
	DummyMetricsSnapshot Snapshot() const;

private:
	std::atomic<int> connected = 0;
	std::atomic<int> loggedIn = 0;
	std::atomic<int> inRoom = 0;
	std::atomic<int> disconnected = 0;

	std::atomic<long long> sent = 0;
	std::atomic<long long> received = 0;
	std::atomic<long long> latencyCount = 0;
	std::atomic<long long> totalLatencyMs = 0;
	std::atomic<long long> maxLatencyMs = 0;
	std::atomic<long long> errors = 0;

	DummyMetricsSnapshot lastSnapshot;
	std::ofstream csv;
};
