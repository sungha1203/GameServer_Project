#include "pch.h"
#include "PacketProcessor.h"
#include "PacketSession.h"

PacketProcessor::PacketProcessor()
{
}

PacketProcessor::~PacketProcessor()
{
	Stop();
}

void PacketProcessor::Start(int threadCnt)
{
	running = true;

	for(int i = 0; i< threadCnt; ++i)
	{
		workers.emplace_back(&PacketProcessor::WorkerThread, this);
	}
}

void PacketProcessor::Stop()
{
	running = false;

	{
		std::lock_guard<std::mutex> lock(queueMutex);

		while(!packetQueue.empty())
		{
			packetQueue.pop();
		}
	}

	cv.notify_all();			// 전체 스레드 깨우고 종료 준비

	for(auto& worker : workers)
	{
		if (worker.joinable())
			worker.join();
	}

	workers.clear();
}

void PacketProcessor::Enqueue(const std::shared_ptr<PacketSession>& session)
{
	if (session == nullptr) return;

	{
		std::lock_guard<std::mutex> lock(queueMutex);
		packetQueue.push(session);
	}

	cv.notify_one();					// 스레드 깨움
}

void PacketProcessor::WorkerThread()
{
	while (running)
	{
		std::shared_ptr<PacketSession> session;
		{
			std::unique_lock<std::mutex> lock(queueMutex);

			cv.wait(lock, [this] { 
				return !packetQueue.empty() || !running; 
				});

			if (packetQueue.empty() && !running)
				return;

			session = packetQueue.front();
			packetQueue.pop();
		}

		// 다른 스레드에서 queue push 가능하게 잠금 해제 후 패킷 처리
		if (session)
		{
			session->ProcessPacket2(); // 패킷 처리
		}
	}
}