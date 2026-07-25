#include "pch.h"
#include "DummyMetrics.h"

#include <algorithm>
#include <iostream>

void DummyMetrics::OnConnected()
{
	connected.fetch_add(1);
}

void DummyMetrics::OnDisconnected()
{
	disconnected.fetch_add(1);
	connected.fetch_sub(1);
}

void DummyMetrics::OnLoggedIn()
{
	loggedIn.fetch_add(1);
}

void DummyMetrics::OnEnterRoom()
{
	inRoom.fetch_add(1);
}

void DummyMetrics::OnSent()
{
	sent.fetch_add(1);
}

void DummyMetrics::OnReceived()
{
	received.fetch_add(1);
}

void DummyMetrics::OnError()
{
	errors.fetch_add(1);
}

void DummyMetrics::OnLatency(long long latencyMs)
{
	latencyCount.fetch_add(1);
	totalLatencyMs.fetch_add(latencyMs);

	long long currentMax = maxLatencyMs.load();
	while (latencyMs > currentMax && !maxLatencyMs.compare_exchange_weak(currentMax, latencyMs))
	{
	}
}

bool DummyMetrics::OpenCsv(const std::string& filePath)
{
	csv.open(filePath, std::ios::out | std::ios::trunc);
	if (!csv.is_open())
		return false;

	csv << "time,connected,loggedIn,inRoom,sendPerSec,recvPerSec,avgLatencyMs,maxLatencyMs,errorCount\n";
	return true;
}

void DummyMetrics::WriteCsvRow(int elapsedSeconds)
{
	const DummyMetricsSnapshot current = Snapshot();
	const long long sentDelta = current.sent - lastSnapshot.sent;
	const long long recvDelta = current.received - lastSnapshot.received;
	const long long latencyDelta = current.latencyCount - lastSnapshot.latencyCount;
	const long long totalLatencyDelta = current.totalLatencyMs - lastSnapshot.totalLatencyMs;
	const long long avgLatency = latencyDelta > 0 ? totalLatencyDelta / latencyDelta : 0;

	lastSnapshot = current;

	const long long errorCount = errors.load();

	if (csv.is_open())
	{
		csv << elapsedSeconds << ','
			<< connected.load() << ','
			<< loggedIn.load() << ','
			<< inRoom.load() << ','
			<< sentDelta << ','
			<< recvDelta << ','
			<< avgLatency << ','
			<< current.maxLatencyMs << ','
			<< errorCount << '\n';
		csv.flush();
	}

	std::cout << "[T+" << elapsedSeconds << "s]"
		<< " connected=" << connected.load()
		<< " loggedIn=" << loggedIn.load()
		<< " inRoom=" << inRoom.load()
		<< " send/s=" << sentDelta
		<< " recv/s=" << recvDelta
		<< " avgLatencyMs=" << avgLatency
		<< " maxLatencyMs=" << current.maxLatencyMs
		<< " errors=" << errorCount
		<< std::endl;
}

DummyMetricsSnapshot DummyMetrics::Snapshot() const
{
	DummyMetricsSnapshot snapshot;
	snapshot.sent = sent.load();
	snapshot.received = received.load();
	snapshot.latencyCount = latencyCount.load();
	snapshot.totalLatencyMs = totalLatencyMs.load();
	snapshot.maxLatencyMs = maxLatencyMs.load();
	return snapshot;
}
