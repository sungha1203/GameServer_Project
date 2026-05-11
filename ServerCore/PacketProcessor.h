#pragma once
#include "pch.h"

class PacketSession;

class PacketProcessor
{
public:
	PacketProcessor();
	~PacketProcessor();

public:
	void										Start(int threadCnt);
	void										Stop();
	void										Enqueue(const std::shared_ptr<PacketSession>& session);

private:
	void										WorkerThread();

private:
	std::mutex									queueMutex;
	std::condition_variable						cv;
	std::queue<std::shared_ptr<PacketSession>>	packetQueue;

	std::vector<std::thread>					workers;
	std::atomic<bool>							running = false;	
};